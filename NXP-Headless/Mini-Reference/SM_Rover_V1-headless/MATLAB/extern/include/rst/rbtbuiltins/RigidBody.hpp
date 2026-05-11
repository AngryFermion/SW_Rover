/* Copyright 2022-2023 The MathWorks, Inc. */
#ifndef RIGID_BODY_HPP
#define RIGID_BODY_HPP
#include "Utils.hpp"

namespace rbtcodegen {

/*! A rigid body joint*/
struct RigidBodyJoint {

    /** Position number
     *
     * The position number signifies the degree of freedom (DOF) associated with a
     * joint's position variables and is the size of its position configuration.
     */
    size_t m_PositionNumber;

    /** Velocity number
     *
     * The velocity number signifies the degree of freedom (DOF) associated with a
     * joint's dynamic (velocity, acceleration, torque, etc.) variables and is the
     * size of its dynamic configuration.
     */
    size_t m_VelocityNumber;

    /** Type of joint
     *
     * The type of joint is used to determine how frames related with this joint
     * will move with respect to each other given the joint's configuration.
     */
    JointType m_Type;

    /** Axis of the joint
     *
     * Axis of a revolute or a prismatic type joint.
     */
    std::array<real64_T, 3> m_JointAxis;

    /// Fixed transform from joint to parent frame
    HomogeneousTransform m_JointToParentTransform;

    /** Motion subspace of a joint.
     *
     * Each joint can  have upto 6 degrees of freedom. The array has a 1 for
     * every degree of freedom affected by a joint.
     */
    std::vector<std::array<real64_T, 6>> m_MotionSubspace;

    /// Maximum velocity degree of freedom of a joint. This corresponds to a floating joint.
    constexpr static const size_t MAX_VELOCITY_NUMBER = 6;

    /// Maximum position degree of freedom of a joint. This corresponds to a floating joint.
    constexpr static const real64_T MAX_POSITION_NUMBER = 7;

    /// Fixed transform from child frame to joint
    HomogeneousTransform m_ChildToJointTransform;

    /** Transform of the child frame to parent frame given joint position.
     *
     * @param[in] q The joint position value.
     * @return tform Homogeneous transformation matrix from child frame to parent
     * frame.
     */
    HomogeneousTransform transformBodyToParent(const std::vector<real64_T>& q) const {
        return mult(mult(m_JointToParentTransform, this->jointTransform(q)),
                    m_ChildToJointTransform);
    }

    /** Relative transform given joint position.
     *
     * @param[in] q The joint position value.
     * @return tform Homogeneous transformation matrix from intermediate child
     * frame to intermediate parent frame.
     */
    HomogeneousTransform jointTransform(const std::vector<real64_T>& q) const {
        HomogeneousTransform out = identity();
        if (m_Type == JointType::REVOLUTE) {
            out = axang2tform(m_JointAxis, q[0]);
        } else if (m_Type == JointType::FIXED) {
            out = identity();
        } else if (m_Type == JointType::FLOATING) {
            auto trvec = [](const std::vector<real64_T>& qfloat) -> std::array<real64_T, 3> {
                return {qfloat[4], qfloat[5], qfloat[6]};
            };
            auto quat = [](const std::vector<real64_T>& qfloat) -> std::array<real64_T, 4> {
                return {qfloat[0], qfloat[1], qfloat[2], qfloat[3]};
            };
            out = mult(trvec2tform(trvec(q)), quat2tform(quat(q)));
        } else if (m_Type == JointType::PRISMATIC) {
            out = trvec2tform({{
                q[0] * m_JointAxis[0],
                q[0] * m_JointAxis[1],
                q[0] * m_JointAxis[2],
            }});
        }
        return out;
    }
};

/*! A rigid body */
struct RigidBody {

    /// Index of the parent body
    int32_T m_ParentIndex;

    /// The joint associated with the body
    RigidBodyJoint m_Joint;
};
} // namespace rbtcodegen

#endif // RIGID_BODY_HPP
