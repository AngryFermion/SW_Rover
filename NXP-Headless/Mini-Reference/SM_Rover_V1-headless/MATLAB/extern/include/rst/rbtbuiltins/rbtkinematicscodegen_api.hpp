/* Copyright 2022-2023 The MathWorks, Inc. */
#ifndef RBTKINEMATICSCODEGEN_API_HPP
#define RBTKINEMATICSCODEGEN_API_HPP
#include "spec.hpp"

typedef struct {
    uint16_T PositionNumber;
    uint16_T VelocityNumber;
    uint8_T JointType;
    real64_T JointAxis[3];
    real64_T JointToParentTransform[16];
    real64_T MotionSubspace[36];
    real64_T ChildToJointTransform[16];
} RigidBodyJoint;

typedef struct {
    int32_T ParentIndex;
    RigidBodyJoint Joint;
} RigidBody;

/** Create a rigid body tree
 *
 * Create a rigid body tree from an array of rigid bodies.
 * @param rbarr An array of rigid bodies
 * @param pdofmap A DOF map indicating the contribution of a body to the
 * position DOF of the rigid body tree
 * @param vdofmap A DOF map indicating the contribution of a body to the
 * velocity DOF of the rigid body tree
 * @param numbodies Number of bodies on the rigid body tree
 * @param pdofmapsize Number of rows in the position DoFMap of a rigid body tree
 * @param vdofmapsize Number of rows in the velocity DoFMap of a rigid body tree
 * @returns rbt A generic pointer to a rigid body tree. Caller is responsible
 * for deallocating this instance.
 */
EXTERN_C RBTCODEGEN_API void* rbtkinematicscodegen_makeRBT(const RigidBody rbarr[],
                                                           const int32_T pdofmap[],
                                                           const int32_T vdofmap[],
                                                           real64_T numbodies,
                                                           real64_T pdofmapsize,
                                                           real64_T vdofmapsize);

/** Forward kinematics routine of a rigid body tree.
 *
 * For a given set of joint configurations, compute the homogeneous
 * transformations of the body frames with respect to the base frame of the
 * rigid body tree.
 *
 * @param[in] rbt An opaque pointer of a rigid body tree
 * @param[in] configarr An array of joint configurations.
 * @param[in] numconfig Number of configurations in configarr
 * @param[out] tforms An array of homogeneous transformations for every
 * configuration in configarr, where each transformation corresponds to a
 * body in the tree.
 */
EXTERN_C RBTCODEGEN_API void rbtkinematicscodegen_forwardKinematics(const void* rbt,
                                                                    const real64_T configarr[],
                                                                    real64_T numconfig,
                                                                    real64_T tforms[]);

/** Geometric Jacobian routine of a rigid body tree.
 *
 * For a given set of joint configurations, compute the geometric Jacobian
 * of the body frames with respect to the base frame of the rigid body tree.
 *
 * @param[in] rbt An opaque pointer of a rigid body tree
 * @param[in] configarr An array of joint configurations.
 * @param[in] numconfig Number of configurations in configarr
 * @param[out] jacout An array of geometric jacobians for every configuration
 * in configarr, where each jacobian corresponds to a body in the tree.
 */
EXTERN_C RBTCODEGEN_API void rbtkinematicscodegen_geometricJacobians(const void* rbt,
                                                                     const real64_T configarr[],
                                                                     real64_T numconfig,
                                                                     real64_T jacout[]);


EXTERN_C RBTCODEGEN_API real64_T rbtkinematicscodegen_getVelocityNumber(const void* rbt);
/** Destruct an instance of a rigid body tree
 *
 * @param rbt An opaque pointer of a rigid body tree
 */
EXTERN_C RBTCODEGEN_API void rbtkinematicscodegen_destruct(const void* rbt);
#endif
