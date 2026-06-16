// Copyright 2024 The MathWorks, Inc.
#ifndef COLLISIONCODEGEN_FUNCTIONAL_API_HPP
#define COLLISIONCODEGEN_FUNCTIONAL_API_HPP
#ifdef BUILDING_LIBMWCOLLISIONFCNCODEGEN
#include "collisioncodegen/collisioncodegen_util.hpp"
#include <ccd/ccd_vec3.h>
#include <ccd/ccd_ccd.h>
#else
#include "collisioncodegen_util.hpp"
#include <ccd_vec3.h>
#include <ccd_ccd.h>
#endif

enum CollisionType { BOX = 0, CYLINDER, SPHERE, CAPSULE, MESH };

typedef struct {
    uint8_T m_Type;
    real64_T m_X;
    real64_T m_Y;
    real64_T m_Z;
    real64_T m_Radius;
    real64_T m_Height;
    const real64_T* m_Vertices;
    uint32_T m_NumVertices;
    real64_T m_Position[3];
    real64_T m_Quaternion[4];
} CollisionGeometryStruct;

EXTERN_C COLLISIONCODEGEN_API ccd_support_fn
collisionGeometryStructSupport(const CollisionGeometryStruct* obj);

EXTERN_C COLLISIONCODEGEN_API int8_T collisioncodegen_intersect2(CollisionGeometryStruct* obj1,
                                                                 const real64_T* vert1,
                                                                 CollisionGeometryStruct* obj2,
                                                                 const real64_T* vert2,
                                                                 real64_T computeDistance,
                                                                 real64_T* p1Vec,
                                                                 real64_T* p2Vec,
                                                                 real64_T* distance);
#endif /* COLLISIONCODEGEN_FUNCTIONAL_API_HPP*/
