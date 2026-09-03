// Copyright 2023-2024 The MathWorks, Inc.
/**
 * @file VDBManager.hpp
 * @brief This file contains the interface for the VDBManager object.
 * @copyright 2024 The MathWorks, Inc.
 */

#ifndef VDBMANAGER_IMPL
#define VDBMANAGER_IMPL

#include <map>
#include <unordered_set>
#include <openvdb/Grid.h>
#include <openvdb/Types.h>
#include <openvdb/math/Transform.h>
#include <openvdb/math/Vec3.h>
#include <openvdb/tools/Interpolation.h>
#include <openvdb/math/Stencils.h>
#include <openvdb/tools/MeshToVolume.h>
#include <openvdb/tools/FastSweeping.h>
#include <openvdb/tools/VolumeToSpheres.h>
#include <openvdb/util/NullInterrupter.h>
#include <openvdb/openvdb.h>
#include <openvdb/io/Stream.h>

#ifdef BUILDING_LIBMWVDBVOLUMECODEGEN
#include "vdbvolumecodegen/NavMeshAdapter.hpp"
#else
#include "NavMeshAdapter.hpp"
#endif

using DistGridType = openvdb::FloatGrid;
using PolyIdxGridType = openvdb::Int32Grid;

namespace nav {
class VDBManager {
  public:
    VDBManager(const double resolution,
               const double truncDist,
               const bool fillInterior,
               const bool fastSweep) {
        m_resolution = static_cast<float>(resolution);
        m_truncDist = static_cast<float>(truncDist);
        m_fillInterior = fillInterior;
        m_fastSweep = fastSweep;
        m_numActiveVoxel = 0u;
        recomputeLimits();
    }

    /**
     * @brief Converts a mesh to a signed distance field grid.
     * @param mesh Input mesh to be discretized.
     * @return Pointer to the resulting distance field grid.
     */
    template <class Mesh>
    DistGridType::Ptr discretizeMesh(const Mesh& mesh) const;

    /**
     * @brief Adds a distance field grid to the manager with a given ID.
     * @param id Identifier for the grid.
     * @param newGrid Pointer to the distance field grid.
     * @return 1.0 if new ID, otherwise 0.0.
     */
    double addElem(const size_t id, DistGridType::Ptr newGrid);

    /**
     * @brief Removes grids associated with a set of IDs from the manager.
     * @param idSet Vector of IDs to remove.
     */
    void removeID(const std::vector<size_t>& idSet);

    /**
     * @brief Applies a transformation to a grid with a given ID.
     * @param id Identifier for the grid.
     * @param pose 4x4 transformation matrix.
     * @return 1.0 if ID found, otherwise 0.0.
     */
    double transformGrid(const size_t id, const double* const pose);

    /**
     * @brief Counts the total number of active voxels in all grids.
     */
    void tallyActiveVoxel(void);

    /**
     * @brief Computes the minimum distances from a set of points to all managed grids.
     * @param pts Points to compute distances to.
     * @param dists Array to store computed distances.
     * @param interpMethod Interpolation method identifier.
     */
    void distance(const std::vector<openvdb::Vec3d>& pts,
                  double* dists,
                  const size_t interpMethod) const;

    /**
     * @brief Computes the gradient of the distance field at a set of points.
     * @param pts Points to compute gradients at.
     * @param grads Array to store computed gradients.
     * @param interpMethod Interpolation method identifier.
     */
    void gradient(const std::vector<openvdb::Vec3d>& pts,
                  double* grads,
                  const size_t interpMethod) const;

    /**
     * @brief Recomputes the world limits based on active grids.
     */
    void recomputeLimits(void);

    /**
     * @brief Retrieves active voxel information from a specified grid.
     * @param id Identifier for the grid.
     * @param ctrs Array to store the centers of active voxels.
     * @param vals Array to store the values of active voxels.
     * @param sizes Array to store the sizes of active voxels.
     */
    void getActiveVoxelFrom(const size_t id, double* ctrs, double* vals, double* sizes) const;

    /**
     * @brief Retrieves the pose of a specified grid.
     * @param id Identifier for the grid.
     * @param poses Array to store the 4x4 transformation matrix representing the grid's pose.
     */
    void getPoseFrom(size_t idSet, double* poses) const;

    /**
     * @brief Retrieves the identifiers of all managed grids.
     * @param idSet Array to store the identifiers of the grids.
     */
    void getID(double* idSet) const;

    /**
     * @brief Serializes the VDBManager and its grids to a binary string.
     * @return A binary string representing the serialized state of the VDBManager.
     */
    std::string serialize(void);

    /**
     * @brief Deserializes a binary string to a VDBManager object.
     * @param vdbManagerString The binary string representing a serialized VDBManager.
     * @return A new VDBManager object reconstructed from the binary string.
     */
    static VDBManager deserialize(std::string& vdbManagerString);

    /**
     * @brief Retrieves the resolution of the VDB grid.
     * @return The resolution value.
     */
    float getRes(void) const {
        return m_resolution;
    };

    /**
     * @brief Retrieves the truncation distance used in the VDB grid.
     * @return The truncation distance value.
     */
    float getTruncDist(void) const {
        return m_truncDist;
    };

    /**
     * @brief Checks if the interior of the VDB grid is filled.
     * @return True if the interior is filled, false otherwise.
     */
    bool getFillInterior(void) const {
        return m_fillInterior;
    };

    /**
     * @brief Checks if the fast sweeping algorithm is used.
     * @return True if fast sweeping is used, false otherwise.
     */
    bool getFastSweep(void) const {
        return m_fastSweep;
    };

    /**
     * @brief Retrieves the number of VDB grids managed by the VDBManager.
     * @return The number of VDB grids.
     */
    size_t getNumVDB(void) const {
        return m_vdbSet.size();
    };

    /**
     * @brief Retrieves the total number of active voxels across all managed VDB grids.
     * @return The total number of active voxels.
     */
    size_t getNumActiveVoxel(void) const {
        return m_numActiveVoxel;
    };

    /**
     * @brief Retrieves the number of active voxels in a specific VDB grid.
     * @param id Identifier for the grid.
     * @return The number of active voxels in the specified grid, or 0 if the grid is not found.
     */
    size_t getNumActiveVoxel(const size_t id) const {
        if (m_vdbSet.find(id) != m_vdbSet.end()) {
            return m_vdbSet.at(id)->activeVoxelCount();
        }
        return 0u;
    }

    /**
     * @brief Retrieves the world limits of the map.
     * @param lims Array to store the minimum and maximum coordinates of the world limits.
     */
    void getMapWorldLimits(double* lims) const {
        lims[0] = m_mapWorldLimits.min().x();
        lims[2] = m_mapWorldLimits.min().y();
        lims[4] = m_mapWorldLimits.min().z();
        lims[1] = m_mapWorldLimits.max().x();
        lims[3] = m_mapWorldLimits.max().y();
        lims[5] = m_mapWorldLimits.max().z();
    };

    /**
     * @brief Provides access to the set of VDB grids managed by the VDBManager.
     * @return A reference to the map containing the VDB grids.
     */
    std::map<size_t, DistGridType::Ptr>& getVDBSet(void) {
        return m_vdbSet;
    }

  private:
    float m_resolution;
    float m_truncDist;
    bool m_fillInterior;
    bool m_fastSweep;
    size_t m_numActiveVoxel;
    openvdb::BBoxd m_mapWorldLimits;
    std::map<size_t, DistGridType::Ptr> m_vdbSet;
};
} // namespace nav

#endif /* VDBMANAGER_IMPL */
