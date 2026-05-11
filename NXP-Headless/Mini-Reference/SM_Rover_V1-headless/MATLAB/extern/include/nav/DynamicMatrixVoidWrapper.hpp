/* Copyright 2022-2023 The MathWorks, Inc. */

#ifndef VOID_WRAPPER_H
#define VOID_WRAPPER_H

#ifdef BUILDING_LIBMWDYNAMICMATRIXCODEGEN
#include "dynmatcodegen/dynamicmatrix_api.hpp"
#include "dynmatcodegen/DynamicMatrix.hpp"
#else
/* To deal with the fact that PackNGo has no include file hierarchy during test */
#include "dynamicmatrix_api.hpp"
#include "DynamicMatrix.hpp"
#endif

#include <functional>

namespace nav {
// Retrieve #elements void-cast DynamicMatrix*
//      Function must cast to appropriate DynamicMatrix<T,N,DEALLOC> before calling getNumel
using NumelFcn = std::function<uint64_T(void*)>;

// Retrieve #dimensions void-cast DynamicMatrix*
//      Function should cast to appropriate DynamicMatrix<T,N,DEALLOC> before calling getMATLABDims
using NumDimFcn = std::function<uint64_T(void*)>;

// Copy size matrix from void-cast DynamicMatrix* into array passed from MATLAB
//      Function should cast to appropriate DynamicMatrix<T,N,DEALLOC> before calling copySizeMatrix
using CopySizeFcn = std::function<void(void*, uint64_T*)>;

// Release the nested matrix and clean up allocated memory contained within
//      Function should cast to appropriate DynamicMatrix<T,N,DEALLOC> before calling delete
using CleanupFcn = std::function<void(void*)>;

// Copy data matrix from void-cast DynamicMatrix* into matrix passed from MATLAB
//      Function should cast to appropriate DynamicMatrix<T,N,DEALLOC> before calling copyData
template <typename Tout>
using copyDataFcn = std::function<void(void*, Tout*)>;

/**
 * @brief A base-class which provides an interface to a type-erased nav::DynamicMatrix object
 *      This object manages a dynamically allocated, type-erased "delegate" - specifically,
 *      it manages a pointer to a DynamicMatrix that has been cast to void.
 *
 *      Handles to DynamicMatrix member functions generated before the type was erased allow this
 * object to redirect calls to the managed pointer without knowledge of the underlying type.
 *
 *      Use nav::createWrapper to convert a nav::DynamicMatrix* to a DynamicMatrixVoidWrapperBase
 *
 *      Example:
 *          MATLAB:
 *              ...
 *              % Create container for dynamically allocated matrix
 *              matWrapper = nav.algs.internal.coder.dynamicMatrixBuildable([inf 3]); % Voxel
 * center(s)
 *
 *              % The C++ function "foo" returns a wrapper around some dynamically allocated C++
 * object representing [inf 3] matrix coder.ceval('foo', ..., coder.ref(matWrapper.Ptr));
 *
 *              % Copy data back to matlab
 *              mat = matWrapper.getData();
 *
 *              % C++ object will get cleaned up automatically when matWrapper goes out of scope
 *
 *          C++:
 *              void foo(..., void** outputPtr)
 *              {
 *                  // Generate a dynamically allocated resource
 *                  auto* data = new std::vector<std::array<double,3>>();
 *
 *                  // Allocate/define populate the data
 *                  ...
 *
 *                  // Wrap the resource in the DynamicMatrix marshaller
 *                  auto* ctrMat = new nav::DynamicMatrix<std::vector<std::array<double, 3>>,
 * 2>(std::move(nav::raw2unique(ctrs)), { ctrs->size(), 3});
 *
 *                  // Generate a new DynamicMatrixVoidWrapperBase and point the void** to it
 *                  nav::createWrapper<double>(ctrMat,*ctrPtr);
 *              }
 */
class DynamicMatrixVoidWrapperBase {
  protected:
    void* m_dynamicMatrix;    // Pointer to a nav::DynamicMatrix that has been cast to void*
    NumelFcn m_numelFcn;     // Handle for calling nav::DynamicMatrix::getNumel()
    NumDimFcn m_numDimFcn;   // Handle for calling nav::DynamicMatrix::getNumDim()
    CopySizeFcn m_sizeFcn;   // Handle for calling nav::DynamicMatrix::copySize()
    CleanupFcn m_CleanupFcn; // Handle for deleting the nav::DynamicMatrix
  public:
    /// @brief Constructor
    /// @param dynamicMat Pointer to a nav::DynamicMatrix
    /// @param fNumel Handle for calling nav::DynamicMatrix::getNumel()
    /// @param fNumDim Handle for calling nav::DynamicMatrix::getNumDim()
    /// @param fSize Handle for calling nav::DynamicMatrix::copySize()
    /// @param fCleanup Handle for deleting the nav::DynamicMatrix
    DynamicMatrixVoidWrapperBase(void* dynamicMat,
                                NumelFcn&& fNumel,
                                NumDimFcn&& fNumDim,
                                CopySizeFcn&& fSize,
                                CleanupFcn&& fCleanup)
        : m_dynamicMatrix(dynamicMat)
        , m_numelFcn(fNumel)
        , m_numDimFcn(fNumDim)
        , m_sizeFcn(fSize)
        , m_CleanupFcn(fCleanup) {}

    /// @brief Virtual destructor called when MATLAB object goes out of scope
    virtual ~DynamicMatrixVoidWrapperBase() {
        m_CleanupFcn(m_dynamicMatrix);
    }

    /// @brief Retrieve number of elements in DynamicMatrix
    /// @return Number of elements
    uint64_T numel(void) const {
        return m_numelFcn(m_dynamicMatrix);
    }

    /// @brief Retrieve number of dimensions in DynamicMatrix
    /// @return Number of dimensions
    uint64_T numDim(void) const {
        return m_numDimFcn(m_dynamicMatrix);
    }

    /// @brief Copy the size of MATLAB-side matrix stored by C++ object
    /// @param szOut Size of matrix in MATLAB
    void size(uint64_T* szOut) const {
        m_sizeFcn(m_dynamicMatrix, szOut);
    }
};

/// @brief A derived version of DynamicMatrixVoidWrapperBase needed for
///     copying data from the DynamicMatrix to linear array of specific datatype
template <typename Tout>
class DynamicMatrixVoidWrapper : public DynamicMatrixVoidWrapperBase {
  protected:
    copyDataFcn<Tout>
        m_dataFcn; // Handle for copying data out of the nav::DynamicMatrix to a linear array
  public:
    /// @brief Constructor
    /// @param dynamicMat Pointer to a nav::DynamicMatrix
    /// @param fNumel Handle for calling nav::DynamicMatrix::getNumel()
    /// @param fNumDim Handle for calling nav::DynamicMatrix::getNumDim()
    /// @param fSize Handle for calling nav::DynamicMatrix::copySize()
    /// @param fCleanup Handle for deleting the nav::DynamicMatrix
    /// @param fData Handle for copying data to linear array
    DynamicMatrixVoidWrapper(void* dynamicMat,
                            NumelFcn&& fNumel,
                            NumDimFcn&& fNumDim,
                            CopySizeFcn&& fSize,
                            CleanupFcn&& fCleanup,
                            copyDataFcn<Tout>&& fData)
        : DynamicMatrixVoidWrapperBase(std::move(dynamicMat),
                                      std::move(fNumel),
                                      std::move(fNumDim),
                                      std::move(fSize),
                                      std::move(fCleanup))
        , m_dataFcn(fData) {}

    virtual ~DynamicMatrixVoidWrapper() {
        // Nothing more to clean
    }

    /// @brief Copy the data from the C++ object to a linear array
    /// @param data Pointer to head of linear array
    void data(Tout* data) const {
        m_dataFcn(m_dynamicMatrix, data);
    }
};

/// @brief Wraps nav::DynamicMatrix in a DynamicMatrixVoidWrapper
/// @param dynamicMatrix Object managing some nested/dynamically-allocated C++ matrix
/// @param[out] wrapper DynamicMatrixVoidWrapper cast to void*
template <typename Tout, typename T, std::size_t N, typename DEALLOC>
void createWrapper(nav::DynamicMatrix<T, N, DEALLOC>*& dynamicMatrix, void*& wrapper) {
    // Create function handles to wrapped object's methods
    NumelFcn f0 = [](void* voidObj) -> uint64_T {
        auto* objPtr = static_cast<nav::DynamicMatrix<T, N, DEALLOC>*>(voidObj);
        return static_cast<uint64_T>(objPtr->getNumel());
    };
    NumDimFcn f1 = [](void* voidObj) -> uint64_T {
        auto* objPtr = static_cast<nav::DynamicMatrix<T, N, DEALLOC>*>(voidObj);
        return static_cast<uint64_T>(objPtr->getNumDim());
    };
    CopySizeFcn f2 = [](void* voidObj, uint64_T* out) {
        auto* objPtr = static_cast<nav::DynamicMatrix<T, N, DEALLOC>*>(voidObj);
        objPtr->copySizeMatrix(out);
    };
    CleanupFcn f3 = [](void* voidObj) {
        if (voidObj != nullptr) {
            auto* objPtr = static_cast<nav::DynamicMatrix<T, N, DEALLOC>*>(voidObj);
            delete objPtr; // sbcheck:ok:allocterms
            objPtr = nullptr;
            voidObj = nullptr;
        }
    };
    copyDataFcn<Tout> f4 = [](void* voidObj, Tout* out) {
        auto* objPtr = static_cast<nav::DynamicMatrix<T, N, DEALLOC>*>(voidObj);
        objPtr->copyData(out);
    };

    // Create typed wrapper
    auto* pDerived = new DynamicMatrixVoidWrapper<Tout>(dynamicMatrix, std::move(f0), std::move(f1),
                                                       std::move(f2), std::move(f3), std::move(f4));

    // Convert to base pointer
    auto* pBase = static_cast<DynamicMatrixVoidWrapperBase*>(pDerived);

    // Return as void pointer
    wrapper = static_cast<void*>(pBase);
}
} // namespace nav

#endif /* VOID_WRAPPER_H */
