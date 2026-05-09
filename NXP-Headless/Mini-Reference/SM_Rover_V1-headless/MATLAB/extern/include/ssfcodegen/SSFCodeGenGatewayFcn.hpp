//  Copyright © 2023 The MathWorks, Inc. All rights reserved.

#ifndef SSFCODEGENGATEWAYFCN_HPP
#define SSFCODEGENGATEWAYFCN_HPP

#include "ssfcodegen/SSFCodeGenTypes.hpp"

#ifdef BUILDING_LIBMWSSFCODEGEN

/* This header is being included by files inside this module */
#include "tmwtypes.h" /* For uint32_T, boolean_T, etc */
#define SSFCODEGENGATEWAYFCN DLL_EXPORT_SYM

#else

#if defined(MATLAB_MEX_FILE) || defined(BUILDING_UNITTEST) || defined(MATLAB_BUILTINS)
#include "tmwtypes.h"
#else
#include "rtwtypes.h"
#endif

#ifndef EXTERN_C
#ifdef __cplusplus
#define EXTERN_C extern "C"
#else
#define EXTERN_C
#endif
#endif

#ifndef SSFCODEGENGATEWAYFCN
#define SSFCODEGENGATEWAYFCN
#endif

#endif


EXTERN_C SSFCODEGENGATEWAYFCN void ssfcg_SetCurrentSimTime(char* uniqueID, double time);

EXTERN_C SSFCODEGENGATEWAYFCN double ssfcg_GetCurrentSimTime(char* uniqueID);

EXTERN_C SSFCODEGENGATEWAYFCN uint32_T ssfcg_GetNumActors(char* uniqueID);

EXTERN_C SSFCODEGENGATEWAYFCN uint32_T ssfcg_GetNumSensors(char* uniqueID);

EXTERN_C SSFCODEGENGATEWAYFCN void ssfcg_GetActorProfiles(char* uniqueID, struct ssfcg_ActorProfile* actorProfiles);

EXTERN_C SSFCODEGENGATEWAYFCN bool ssfcg_IsConnectedToScenario(char* uniqueID);

EXTERN_C SSFCODEGENGATEWAYFCN void ssfcg_GetSSFReset(char* uniqueID);

/* APIs for ActorPoses*/
EXTERN_C SSFCODEGENGATEWAYFCN void ssfcg_GetTargetPoses(char* uniqueID, uint32_T sensorID, double* filterIDs, double numFilterIDs, struct ssfcg_ActorPose* tgtPoses);

EXTERN_C SSFCODEGENGATEWAYFCN void ssfcg_GetActorPoses(char* uniqueID, double* actorIDs, double numIDs, struct ssfcg_ActorPose* actorPoses);

EXTERN_C SSFCODEGENGATEWAYFCN void ssfcg_GetActorPosesInRange(char* uniqueID, uint32_T sensorID, struct ssfcg_ActorPose* actorPoses);

EXTERN_C SSFCODEGENGATEWAYFCN void ssfcg_GetTargetPosesinLOS(char* uniqueID, uint32_T sensorID, double* filterIDs, double numFilterIDs, struct ssfcg_ActorPose* actorPoses);

EXTERN_C SSFCODEGENGATEWAYFCN void ssfcg_GetLaneBoundariesInRange(char* uniqueID, uint32_T sensorID, uint32_T lbOption, struct ssfcg_LaneBoundary* lbs);

/* APIs for RayIntersections*/
EXTERN_C SSFCODEGENGATEWAYFCN void* ssfcg_GetRayIntersectionsRef(char* uniqueID, uint32_T sensorID);

EXTERN_C SSFCODEGENGATEWAYFCN uint32_T ssfcg_GetNumIntersections(void* intersectionSetPtr);

EXTERN_C SSFCODEGENGATEWAYFCN void ssfcg_GetAllIntersectionsPositions(void* intersectionSetPtr, double* intersections);

EXTERN_C SSFCODEGENGATEWAYFCN void ssfcg_GetAllIntersectionsPosInHostCoord(void* intersectionSetPtr, double* intersections);

EXTERN_C SSFCODEGENGATEWAYFCN void ssfcg_GetAllIntersectionsPosInSensorCoord(void* intersectionSetPtr, double* intersections);

EXTERN_C SSFCODEGENGATEWAYFCN void ssfcg_GetNormals(void* intersectionSetPtr, double* normals);

EXTERN_C SSFCODEGENGATEWAYFCN void ssfcg_GetDistances(void* intersectionSetPtr, double* distances);

EXTERN_C SSFCODEGENGATEWAYFCN void ssfcg_GetTargetIds(void* intersectionSetPtr, double* targetIds);

EXTERN_C SSFCODEGENGATEWAYFCN void* ssfcg_GetTargetMaterialRef(void* intersectionSetPtr);

EXTERN_C SSFCODEGENGATEWAYFCN void ssfcg_GetTargetMaterialsAttributeString(void* targetMaterialPtr, char* attribute, char* values);

EXTERN_C SSFCODEGENGATEWAYFCN void ssfcg_GetTargetMaterialsAttributeDoubleArray(void* targetMaterialPtr, char* attribute, double* values);

/* Path Set APIs */
EXTERN_C SSFCODEGENGATEWAYFCN uint32_T ssfcg_GetNumPathSets(char* uniqueID, uint32_T sensorID);

EXTERN_C SSFCODEGENGATEWAYFCN void ssfcg_GetPathSets(char* uniqueID, uint32_T sensorID, uint32_T pathSetIdx, uint32_T* numPaths);

EXTERN_C SSFCODEGENGATEWAYFCN void ssfcg_GetAngleOfDeparture(char* uniqueID, uint32_T sensorID, uint32_T pathSetIdx, uint32_T pathIdx, double* angleOfDep);

EXTERN_C SSFCODEGENGATEWAYFCN void ssfcg_GetAngleOfArrival(char* uniqueID, uint32_T sensorID, uint32_T pathSetIdx, uint32_T pathIdx, double* angleOfArr);

EXTERN_C SSFCODEGENGATEWAYFCN void ssfcg_GetPathLength(char* uniqueID, uint32_T sensorID, uint32_T pathSetIdx, uint32_T pathIdx, double* pathLength);

EXTERN_C SSFCODEGENGATEWAYFCN void ssfcg_GetShortestPathLength(char* uniqueID, uint32_T sensorID, uint32_T pathSetIdx, uint32_T* pathIdx, double* shortestPathLength);

EXTERN_C SSFCODEGENGATEWAYFCN uint32_T ssfcg_GetNumPathIntersectionPositions(char* uniqueID, uint32_T sensorID, uint32_T pathSetIdx, uint32_T pathIdx);

EXTERN_C SSFCODEGENGATEWAYFCN void ssfcg_GetPathIntersectionPositions(char* uniqueID, uint32_T sensorID, uint32_T pathSetIdx, uint32_T pathIdx, double* positions);

EXTERN_C SSFCODEGENGATEWAYFCN void ssfcg_GetPathIntersectionVelocities(char* uniqueID, uint32_T sensorID, uint32_T pathSetIdx, uint32_T pathIdx, double* velocities);

EXTERN_C SSFCODEGENGATEWAYFCN uint32_T ssfcg_GetNumTargetActors(char* uniqueID, uint32_T sensorID, uint32_T pathSetIdx, uint32_T pathIdx);

EXTERN_C SSFCODEGENGATEWAYFCN void ssfcg_GetTargetActors(char* uniqueID, uint32_T sensorID, uint32_T pathSetIdx, uint32_T pathIdx, double* tgtActors);

EXTERN_C SSFCODEGENGATEWAYFCN void ssfcg_GetPathLengthRates(char* uniqueID, uint32_T sensorID, uint32_T pathSetIdx, uint32_T pathIdx, double* lengthRates);

EXTERN_C SSFCODEGENGATEWAYFCN void ssfcg_GetPathLoss(char* uniqueID, uint32_T sensorID, uint32_T pathSetIdx, uint32_T pathIdx, double* loss, double* shift);

#endif /* SSFCODEGENGATEWAYFCN_HPP */