// Copyright 2023-2024 The MathWorks, Inc.

/**
 * This file contains a lightweight wrapper which adapts a simple vertice/face collection to OpenVDB
 * algorithms
 */

#ifndef NAV_MESH_ADAPTER
#define NAV_MESH_ADAPTER

#ifdef BUILDING_LIBMWVDBVOLUMECODEGEN
#include "vdbvolumecodegen/OpenVDBNAVUtils.hpp"
#else
#include "OpenVDBNAVUtils.hpp"
#endif

namespace nav {
/// @brief An adaptor which converts vertices defined w.r.t a geometry's LOCAL frame
/// (herein defined as "world"-space) to axially-aligned grid coordinates
/// (herein defined as "index"-space). Once this "local" mesh has been discretized,
/// the geometry's world->local transformation may be applied to the TSDF, situating
/// it atop the mesh in world coordinates.
template <typename Vert_T, typename Face_T, typename Sz_T>
struct MeshDataAdapter {
    openvdb::math::Transform::Ptr m_tform;
    const Vert_T* m_V;
    const Face_T* m_F;
    size_t m_nVert;
    size_t m_nFace;

    MeshDataAdapter(openvdb::math::Transform::Ptr tform,
                    const Sz_T nV,
                    const Vert_T* const vertices,
                    const Sz_T nF,
                    const Face_T* const faces) {
        m_tform = tform->copy();
        m_nVert = static_cast<size_t>(nV);
        m_nFace = static_cast<size_t>(nF);
        m_V = vertices;
        m_F = faces;
    }
    size_t polygonCount(void) const {
        return m_nFace;
    }
    size_t pointCount(void) const {
        return m_nVert;
    }
    size_t vertexCount(size_t n) const {
        return 3u;
    }
    void getIndexSpacePoint(size_t n, size_t v, openvdb::Vec3d& pos) const {
        uint32_t ptIdx = static_cast<uint32_t>(m_F[n + m_nFace * v] - 1);
        pos = m_tform->worldToIndex({static_cast<float>(m_V[ptIdx]),
                                     static_cast<float>(m_V[ptIdx + m_nVert]),
                                     static_cast<float>(m_V[ptIdx + m_nVert * 2])});
    }
};
} // namespace nav

#endif /* NAV_MESH_ADAPTER */
