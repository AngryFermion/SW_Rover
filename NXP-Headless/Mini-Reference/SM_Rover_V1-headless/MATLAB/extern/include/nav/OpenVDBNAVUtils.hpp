// Copyright 2023-2024 The MathWorks, Inc.
#ifndef OPENVDB_NAV_UTILS
#define OPENVDB_NAV_UTILS

#include <openvdb/Grid.h>
#include <openvdb/Types.h>
#include <openvdb/math/Transform.h>
#include <openvdb/math/Stencils.h>

namespace nav {

    inline openvdb::math::Transform::Ptr createLocalTransform(float resolution,
                                                              bool voxelCentered = true) {
        // Create base transform for map of given resolution
        auto tform = openvdb::math::Transform::createLinearTransform(1.0f / resolution);

        if (voxelCentered) {
            // Offset voxel index location by half-cell
            double halfStep = 1.0 / resolution / 2.0;
            openvdb::Vec3d halfCellSize(halfStep, halfStep, halfStep);
            tform->postTranslate(halfCellSize);
        }

        return tform;
    }

    template<class VDBGridTypePtr>
    inline void applyPoseToGrid(VDBGridTypePtr gridPtr, float resolution, double const* const pose, bool voxelCentered = true) {
        // Create local cell-centered transformation
        openvdb::math::Transform::Ptr tform = createLocalTransform(resolution, voxelCentered);

        // 4x4 being passed col-major from MATLAB, but openvdb applies 4x4 transforms
        // via post-multiplication. We can therefore read/write OpenVDB matrices without
        // transposing.
        auto poseMat = openvdb::math::Mat4d(pose);
        auto cellCtrMap = tform->baseMap()->getAffineMap()->getMat4();
        tform = openvdb::math::Transform::createLinearTransform(cellCtrMap * poseMat);
        gridPtr->setTransform(tform);
    }

    /// @brief Default limits for an empty VDB map
    constexpr std::array<double, 6> DEFAULT_LIMITS{0, 0, 0, 0, 0, 0};

    // A weighting function for merging new distance values with previously stored values.
    // Per vdbfusion library, weight = 1 results in simply returning the mean.
    inline std::function<float(float)> vdbfusionMeanWeightFcn = [](float /* unused */) { return 1.0f; };

    /// @brief /// @brief Computes gradient at specified world-XYZ points using specified stencil
    /// @tparam Stencil_T Family of stencil objects found in openvdb::math::
    /// @param stencil Stencil object for computing gradient. Contains reference to a given OpenVDB grid
    /// object
    /// @param tformPtr Transformation used for converting points from world<->index space
    /// @param numPt Number of query points in ptData
    /// @param ptData XYZ world coordinates in row-major array
    /// @param grad XYZ gradient vectors in row-major array
    template <typename Stencil_T>
    static void computeGradient(Stencil_T* stencil,
                                openvdb::math::Transform::Ptr tformPtr,
                                const size_t numPt,
                                const double* const ptData,
                                double* grad) {
        for (size_t i = 0; i < numPt; i++) {
            // Move stencil to index-space coordinate and compute gradient
            auto ijk = tformPtr->worldToIndex({ptData[i], ptData[i + numPt], ptData[i + numPt * 2]});
            stencil->moveTo(ijk);
            openvdb::math::Vec3d gVec = stencil->gradient(ijk);
            grad[i] = gVec.x();
            grad[i + numPt] = gVec.y();
            grad[i + numPt * 2] = gVec.z();
        }
    }
} // namespace nav

#endif /* OPENVDB_NAV_UTILS */
