/* Copyright 2022-2023 The MathWorks, Inc. */

/**
 * @file
 * External C-API interfaces for DynamicMatrix helper.
 * To fully support code generation, note that this file needs to be fully
 * compliant with the C89/C90 (ANSI) standard.
 */

#ifndef DYNAMICMATRIX_CODEGEN_API_H_
#define DYNAMICMATRIX_CODEGEN_API_H_

#ifdef BUILDING_LIBMWDYNAMICMATRIXCODEGEN
#include "dynmatcodegen/dynamicmatrix_util.hpp"
#else
/* To deal with the fact that PackNGo has no include file hierarchy during test */
#include "dynamicmatrix_util.hpp"
#endif

// Macro redefinitions for customers calling R2023a and earlier API, which prefixed all API 
// with "nested" rather than "dynamic"
#define nestedmatrixcodegen_getNumel(OBJ)               dynamicmatrixcodegen_getNumel(OBJ)
#define nestedmatrixcodegen_getNumDimensions(OBJ)       dynamicmatrixcodegen_getNumDimensions(OBJ)
#define nestedmatrixcodegen_getMATLABSize(OBJ,SZ)       dynamicmatrixcodegen_getMATLABSize(OBJ,SZ)
#define nestedmatrixcodegen_destruct(OBJ)               dynamicmatrixcodegen_destruct(OBJ)
#define nestedmatrixcodegen_retrieve_REAL64(OBJ,DEST)   dynamicmatrixcodegen_retrieve_REAL64(OBJ,DEST)
#define nestedmatrixcodegen_retrieve_REAL32(OBJ,DEST)   dynamicmatrixcodegen_retrieve_REAL32(OBJ,DEST)
#define nestedmatrixcodegen_retrieve_UINT64(OBJ,DEST)   dynamicmatrixcodegen_retrieve_UINT64(OBJ,DEST)
#define nestedmatrixcodegen_retrieve_UINT32(OBJ,DEST)   dynamicmatrixcodegen_retrieve_UINT32(OBJ,DEST)
#define nestedmatrixcodegen_retrieve_INT64(OBJ,DEST)    dynamicmatrixcodegen_retrieve_INT64(OBJ,DEST)
#define nestedmatrixcodegen_retrieve_INT32(OBJ,DEST)    dynamicmatrixcodegen_retrieve_INT32(OBJ,DEST)
#define nestedmatrixcodegen_retrieve_BOOLEAN(OBJ,DEST)  dynamicmatrixcodegen_retrieve_BOOLEAN(OBJ,DEST)

/**
 * @brief Retrieve number of elements in array stored by DynamicMatrixVoidWrapperBase pointer
 *
 * @param voidWrapper void*-cast ptr to DynamicMatrixVoidWrapperBase*
 * @return numel Total number of elements in stored matrix
 */
EXTERN_C DYNAMICMATRIX_CODEGEN_API uint64_T dynamicmatrixcodegen_getNumel(void* voidWrapper);

/**
 * @brief Retrieve number of dimensions in array stored by DynamicMatrixVoidWrapperBase pointer
 *
 * @param voidWrapper void*-cast ptr to DynamicMatrixVoidWrapperBase*
 * @return numDim Dimensions of stored matrix in MATLAB
 */
EXTERN_C DYNAMICMATRIX_CODEGEN_API uint64_T dynamicmatrixcodegen_getNumDimensions(void* voidWrapper);

/**
 * @brief Retrieve size of array stored by DynamicMatrixVoidWrapperBase pointer
 *
 * @param voidWrapper void*-cast ptr to DynamicMatrixVoidWrapperBase*
 * @param[out] sz A 1xN array storing the size of the data matrix stored by the DynamicArray
 */
EXTERN_C DYNAMICMATRIX_CODEGEN_API void dynamicmatrixcodegen_getMATLABSize(void* voidWrapper,
                                                                    uint64_T* sz);

/**
 * @brief Delete pointer to DynamicMatrixVoidWrapperBase
 *
 * @param voidWrapper void*-cast ptr to DynamicMatrixVoidWrapperBase*
 */
EXTERN_C DYNAMICMATRIX_CODEGEN_API void dynamicmatrixcodegen_destruct(void* voidWrapper);

/**
 * The following API will copy the data out of the nested container
 * and cast it to the type defined by the function.
 *
 *      NOTE: The API used MUST MATCH the type used to construct the
 *            DynamicMatrixVoidWrapper<type>* (passed here as void*)
 */

/**
 * @brief Retrieve data stored by DynamicMatrixVoidWrapper<double>*
 *
 * @param voidWrapper void*-cast ptr to DynamicMatrixVoidWrapper<double>*
 * @param[out] dest Allocated array of DOUBLES which will receive the values stored in voidWrapper
 */
EXTERN_C DYNAMICMATRIX_CODEGEN_API void dynamicmatrixcodegen_retrieve_REAL64(void* voidWrapper,
                                                                      real64_T* dest);

/**
 * @brief Retrieve data stored by DynamicMatrixVoidWrapper<float>*
 *
 * @param voidWrapper void*-casted ptr to DynamicMatrixVoidWrapper<float>*
 * @param[out] dest Allocated array of SINGLES which will receive the values stored in voidWrapper
 */
EXTERN_C DYNAMICMATRIX_CODEGEN_API void dynamicmatrixcodegen_retrieve_REAL32(void* voidWrapper,
                                                                      real32_T* dest);

/**
 * @brief Retrieve data stored by DynamicMatrixVoidWrapper<uint64>*
 *
 * @param voidWrapper void*-casted ptr to DynamicMatrixVoidWrapper<uint64>*
 * @param[out] dest Allocated array of UINT64 which will receive the values stored in voidWrapper
 */
EXTERN_C DYNAMICMATRIX_CODEGEN_API void dynamicmatrixcodegen_retrieve_UINT64(void* voidWrapper,
                                                                      uint64_T* dest);

/**
 * @brief Retrieve data stored by DynamicMatrixVoidWrapper<uint32>*
 *
 * @param voidWrapper void*-casted ptr to DynamicMatrixVoidWrapper<uint32>*
 * @param[out] dest Allocated array of UINT32 which will receive the values stored in voidWrapper
 */
EXTERN_C DYNAMICMATRIX_CODEGEN_API void dynamicmatrixcodegen_retrieve_UINT32(void* voidWrapper,
                                                                      uint32_T* dest);

/**
 * @brief Retrieve data stored by DynamicMatrixVoidWrapper<int64>*
 *
 * @param voidWrapper void*-casted ptr to DynamicMatrixVoidWrapper<int64>*
 * @param[out] dest Allocated array of INT64 which will receive the values stored in voidWrapper
 */
EXTERN_C DYNAMICMATRIX_CODEGEN_API void dynamicmatrixcodegen_retrieve_INT64(void* voidWrapper,
                                                                     int64_T* dest);

/**
 * @brief Retrieve data stored by DynamicMatrixVoidWrapper<int32>*
 *
 * @param voidWrapper void*-casted ptr to DynamicMatrixVoidWrapper<int32>*
 * @param[out] dest Allocated array of INT32 which will receive the values stored in voidWrapper
 */
EXTERN_C DYNAMICMATRIX_CODEGEN_API void dynamicmatrixcodegen_retrieve_INT32(void* voidWrapper,
                                                                     int32_T* dest);

/**
 * @brief Retrieve data stored by DynamicMatrixVoidWrapper<bool>*
 *
 * @param voidWrapper void*-casted ptr to DynamicMatrixVoidWrapper<bool>*
 * @param[out] dest Allocated array of BOOLEANS which will receive the values stored in voidWrapper
 */
EXTERN_C DYNAMICMATRIX_CODEGEN_API void dynamicmatrixcodegen_retrieve_BOOLEAN(void* voidWrapper,
                                                                       boolean_T* dest);

#endif /* DYNAMICMATRIX_CODEGEN_API_H_ */
