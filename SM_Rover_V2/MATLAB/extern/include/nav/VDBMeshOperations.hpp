// Copyright 2024 The MathWorks, Inc.
/**
 * @file VDBMeshOperations.hpp
 * @brief This file contains the mesh-specific operations on the VDBManager.
 * @copyright 2024 The MathWorks, Inc.
 */
#ifndef VDBMESHOPERATIONS_IMPL
#define VDBMESHOPERATIONS_IMPL

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
#include "vdbvolumecodegen/VDBManager.hpp"
#else
#include "NavMeshAdapter.hpp"
#include "VDBManager.hpp"
#endif


namespace nav {
    using DistGridType = openvdb::FloatGrid;
    using PolyIdxGridType = openvdb::Int32Grid;

    /**
     * @brief Creates a mesh adapter from given mesh data.
     * 
     * @tparam Data_T Type of the mesh data.
     * @tparam Size_T Type of the size specifier.
     * @param resolution The resolution of the voxels (in cells/meters).
     * @param pose Pointer to an array containing the SE3 pose of the mesh.
     * @param nVert Number of vertices in the mesh.
     * @param vertices Pointer to an array containing the vertices of the mesh.
     * @param nFace Number of faces in the mesh.
     * @param faces Pointer to an array containing the faces of the mesh.
     * @return MeshDataAdapter<Data_T, Data_T, Size_T> A mesh adapter for the given mesh data.
     */
    template <class Data_T, class Size_T>
    MeshDataAdapter<Data_T, Data_T, Size_T> createMesh(float resolution, Data_T const* const pose, const Size_T nVert,
        Data_T const* const vertices, const Size_T nFace, Data_T const* const faces);

    /**
     * @brief Discretizes a mesh into a signed distance field grid.
     * 
     * @tparam Mesh Type of the mesh.
     * @param mesh The mesh to discretize.
     * @param truncDist The truncation distance for the TSDF map (in meters).
     * @param resolution The resolution of the voxels (in cells/meters).
     * @param fillInterior Flag indicating whether to fill the interior of the mesh with distance values.
     * @param fastSweep Flag indicating whether to use a fast sweeping algorithm for TSDF updates.
     * @return DistGridType::Ptr A pointer to a grid containing the discretized mesh.
     */
    template <class Mesh>
    DistGridType::Ptr discretizeMesh(const Mesh& mesh, float truncDist, float resolution, bool fillInterior, bool fastSweep);

    /**
     * @brief Adds a mesh to the VDBManager with the given ID and applies a pose.
     * @param mgr Reference to the VDBManager.
     * @param id The ID to associate with the new mesh.
     * @param pose Pointer to the array representing a transformation matrix (1x16 array of doubles).
     * @param nVert The number of vertices (size_t).
     * @param vertices Pointer to the vertices data (Mx3 array of doubles, representing M vertices in 3D space).
     * @param nFace The number of faces (size_t).
     * @param faces Pointer to the face indices data (Nx3 array of doubles, representing N faces with indices to vertices).
     * @return double Returns 1.0 if a new ID was added, 0.0 otherwise.
     */
    double addMesh(VDBManager& mgr,
        const size_t id,
        const double* const pose,
        const size_t nVert,
        const double* const vertices,
        const size_t nFace,
        const double* const faces);

    /**
     * @brief Applies a transformation pose to the given VDB grid.
     * @param grid Pointer to the VDB grid to transform.
     * @param resolution The resolution of the VDB grid (in cells/meters).
     * @param pose Pointer to the array representing a transformation matrix (1x16 array of doubles).
     */
    void transformGrid(DistGridType::Ptr grid, float resolution, const double* const pose);

} // namespace nav

#endif // VDBMESHOPERATIONS_IMPL
