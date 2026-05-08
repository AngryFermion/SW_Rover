/* Copyright 2022-2023 The MathWorks, Inc. */
#ifndef SSFCODEGENTYPES_HPP
#define SSFCODEGENTYPES_HPP

/* This header is being included by files inside this module */
#include "tmwtypes.h" /* For uint32_T, boolean_T, etc */

typedef struct ssfcg_ActorPose {
    double ActorID;
    double ClassID;
    double Position[3];
    double Velocity[3];
    double Roll;
    double Pitch;
    double Yaw;
    double AngularVelocity[3];
} ssfcg_ActorPose;

typedef struct ssfcg_ActorProfile {
    double ActorID;
    double ClassID;
    double Length;
    double Width;
    double Height;
    //double FrontOverhang;
    //double RearOverhang;
    double MeshVertices[100];
    double MeshFaces[100];
    double OriginOffset[3];
    double RCSPattern[4];
    double RCSAzimuthAngles[2];
    double RCSElevationAngles[2];
} ssfcg_ActorProfile;

typedef struct ssfcg_LaneBoundary {
    char* ID;
    double Coordinates[300]; // Set 100 as max number of points along the lane boundary.
    double Curvature[100];
    double CurvatureDerivative[100];
    double HeadingAngle;
    double LateralOffset;
    int BoundaryType;
    double Strength;
    double Width;
    double Length;
    double Space;
} ssfcg_LaneBoundary;

#endif /* SSFCODEGENTYPES_HPP */