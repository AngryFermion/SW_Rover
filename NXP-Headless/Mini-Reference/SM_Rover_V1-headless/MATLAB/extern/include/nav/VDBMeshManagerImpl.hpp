// Copyright 2023-2024 The MathWorks, Inc.
/**
 * @file VDBMeshManagerImpl.hpp
 * @brief This file contains the interface of the MCOS wrapper for the VDBManager object.
 * @copyright 2024 The MathWorks, Inc.
 */

#ifndef VDBMESHMANAGER_IMPL
#define VDBMESHMANAGER_IMPL

#include "vdbvolumebuiltin/vdbvolume_mcosutils.hpp"
#include "vdbvolumebuiltin/vdbvolumebuiltin_util.hpp"

/**
 * @brief Checks out the NAV license.
 * 
 * Throws an exception if the license cannot be checked out.
 */
namespace nav {
    void checkoutNAV(void);

    // Forward declarations
    class VDBManager;

/**
 * @brief Constructs a new VDBMeshManagerImpl object with given parameters.
 * 
 * @param resolution The resolution of the voxel grid.
 * @param truncDist The truncation distance for the SDF.
 * @param fillInterior Specifies whether to fill the interior of the mesh.
 * @param fastSweep Specifies whether to use a fast sweeping method.
 */
class VDBVOLUMEBUILTIN_EXPORT_CLASS VDBMeshManagerImpl {
  public:
    VDBMeshManagerImpl(const double resolution,
                       const double truncDist,
                       const bool fillInterior,
                       const bool fastSweep);

    /**
     * @brief Gets the number of VDBs managed by the VDBMeshManager.
     * 
     * @return The number of VDBs.
     */
    double getNumVDB(void) const;

    /**
     * @brief Gets the truncation distance for the SDF.
     * 
     * @return The truncation distance.
     */
    double getTruncDist(void) const;

    /**
     * @brief Gets the resolution of the voxel grid.
     * 
     * @return The resolution of the grid (cells/meter).
     */
    double getResolution(void) const;

    /**
     * @brief Gets the number of active voxels in the VDB volume.
     * 
     * @return The number of active voxels.
     */
    double getNumActiveVoxel(void) const;

    /**
     * @brief Gets the active bounding box of the VDB volume.
     * 
     * @return An mxArrayProxy representing the active bounding box.
     */
    mxArrayProxy getActiveBoundingBox(void) const;

    /**
     * @brief Gets the IDs of the VDB volumes managed by the VDBMeshManager.
     * 
     * @return An mxArrayProxy containing the IDs.
     */
    mxArrayProxy getID(void) const;

    /**
     * @brief Updates the poses of the VDB volumes managed by the VDBMeshManager.
     * 
     * @param poseStruct An mxArrayProxy containing the poses to update.
     * @return An mxArrayProxy indicating the success of each pose update.
     */
    mxArrayProxy updatePoses(const mxArrayProxy& poseStruct);

    /**
     * @brief Removes the VDB volumes with specified IDs from the VDBMeshManager.
     * 
     * @param idSet An mxArrayProxy containing the IDs to remove.
     */
    void removeID(const mxArrayProxy& idSet);

    /**
     * @brief Retrieves the active voxels information from all VDB volumes managed by the VDBMeshManager.
     *
     * This method compiles information about the active voxels from each VDB volume into a structured array. Each element
     * of the structured array corresponds to a single VDB volume and contains the following fields:
     * - 'ID': A scalar double representing the unique identifier of the VDB volume.
     * - 'Centers': An N-by-3 matrix where each row contains the (x, y, z) coordinates of an active voxel's center.
     * - 'Distances': An N-by-1 column vector containing the signed distance field (SDF) values of the active voxels.
     * - 'Sizes': An N-by-1 column vector containing the size of each active voxel.
     *
     * Here, N is the number of active voxels in the respective VDB volume.
     *
     * @return An mxArrayProxy structure representing a structured array with active voxels data for each VDB volume.
     */
    mxArrayProxy getActiveVoxels(void) const;

    /**
     * @brief Retrieves the pose matrices of all VDB volumes managed by the VDBMeshManager.
     *
     * This method compiles the pose information for each VDB volume into a structured array. Each element of the array
     * corresponds to a single VDB volume and contains the following fields:
     * - 'ID': A scalar double representing the unique identifier of the VDB volume.
     * - 'Pose': A 4-by-4 matrix representing the transformation matrix (pose) of the VDB volume in homogeneous coordinates.
     *
     * The 'Pose' matrix encodes the position and orientation of the VDB volume in a global reference frame.
     *
     * @return An mxArrayProxy structure representing a structured array with the ID and pose data for each VDB volume.
     */
    mxArrayProxy getPoses(void) const;

    /**
     * @brief Calculates minimum distances from points to VDB meshes.
     * @param pts Points as an N-by-3 mxArrayProxy.
     * @param interpMethod Interpolation method identifier.
     * @return Distances as an N-by-1 mxArrayProxy.
     */
    mxArrayProxy distance(const mxArrayProxy& pts, const size_t interpMethod);

    /**
     * @brief Calculates gradients of the distance field at points.
     * @param pts Points as an N-by-3 mxArrayProxy.
     * @param interpMethod Interpolation method identifier.
     * @return Gradients as an N-by-3 mxArrayProxy.
     */
    mxArrayProxy gradient(const mxArrayProxy& pts, const size_t interpMethod);

    /**
     * @brief Serializes VDBMeshManager state to a MATLAB uint8 array.
     * @return Serialized data as an mxArrayProxy storing a 1-by-N uint8 array.
     */
    mxArrayProxy serialize(void);

    /**
     * @brief Restores VDBMeshManager state from serialized data.
     * @param serializedMgrData Serialized data as an mxArrayProxy storing a 1-by-N uint8 array.
     */
    void deserialize(const mxArrayProxy& serializedMgrData);

    /**
     * @brief Retrieves a shared pointer to the VDBMeshManager.
     * @return Shared pointer to the VDBMeshManager.
     */
    std::shared_ptr<void> getManager(void) {
        return m_VDBManager;
    }

    /**
     * @brief Retrieves a shared pointer to the VDBMeshManager (const version).
     * @return Shared pointer to the VDBMeshManager.
     */
    std::shared_ptr<void> getManager(void) const {
        return m_VDBManager;
    }

  protected:
    std::shared_ptr<void> m_VDBManager;
};

} // namespace nav

#endif
