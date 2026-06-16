/* Copyright 2020-2023 The MathWorks, Inc. */
#ifdef SUPPORTS_PRAGMA_ONCE
#pragma once
#endif

#ifndef SSEDATASERVICESRTW_H
#define SSEDATASERVICESRTW_H

#include "tmwtypes.h"

#include "SSEDataServicesRTW_spec.h"

/* The data type of the user-defined data field */
typedef enum _field_data_type {
    TypeDouble = 0,
    TypeUInt16,
    TypeInt32,
    TypeUInt32,
    TypeString,
    TypeLogical,
    NumTypes
} FieldDataType;

/*
 * These functions act as a bridge to call the c-api of liveio from rapid accel target.
 * The call from c to c++ takes place here, 'Codegen' postfixed files in live/core
 * convert the c parameters to c++ objects and call the library functions.
 */

/*------------------------- SysObj Actor Functions --------------------------*/

SSE_DATASERVICES_RTW_EXPORT_EXTERN_C
uint64_T sseSysObjInitRapid(const char* pkgName, const char* mdlName);


SSE_DATASERVICES_RTW_EXPORT_EXTERN_C
void sseSysObjInitGRT(uint64_T actorID, const char* pkgName, const char* mdlName);


SSE_DATASERVICES_RTW_EXPORT_EXTERN_C
void sseSysObjResetStep();


SSE_DATASERVICES_RTW_EXPORT_EXTERN_C
double sseSysObjGetStepSize();


SSE_DATASERVICES_RTW_EXPORT_EXTERN_C
double sseSysObjGetMaxSimulationTime();


SSE_DATASERVICES_RTW_EXPORT_EXTERN_C
size_t sseSysObjGetNumActors();


SSE_DATASERVICES_RTW_EXPORT_EXTERN_C
uint64_T sseSysObjGetActorID(size_t idx);


SSE_DATASERVICES_RTW_EXPORT_EXTERN_C
size_t sseSysObjGetNumChildrenActors(uint64_T actorID);


SSE_DATASERVICES_RTW_EXPORT_EXTERN_C
uint64_T sseSysObjGetChildActorID(uint64_T actorID, size_t idx);


SSE_DATASERVICES_RTW_EXPORT_EXTERN_C
uint64_T sseSysObjGetParentActorID(uint64_T actorID);


SSE_DATASERVICES_RTW_EXPORT_EXTERN_C
void sseSysObjReportDiagnostic(const char* msgType, const char* msg);


SSE_DATASERVICES_RTW_EXPORT_EXTERN_C
void sseSysObjActorReportDiagnostic(uint64_T actorID, const char* msgType, const char* msg);


SSE_DATASERVICES_RTW_EXPORT_EXTERN_C
void sseSysObjGetPose(uint64_T actorID, double* mat4by4);


SSE_DATASERVICES_RTW_EXPORT_EXTERN_C
void sseSysObjSetPose(uint64_T actorID, double* mat4by4);


SSE_DATASERVICES_RTW_EXPORT_EXTERN_C
void sseSysObjGetVelocity(uint64_T actorID, double* vec1by3);


SSE_DATASERVICES_RTW_EXPORT_EXTERN_C
void sseSysObjSetVelocity(uint64_T actorID, double* vec1by3);


SSE_DATASERVICES_RTW_EXPORT_EXTERN_C
void sseSysObjGetAngularVelocity(uint64_T actorID, double* vec1by3);


SSE_DATASERVICES_RTW_EXPORT_EXTERN_C
void sseSysObjSetAngularVelocity(uint64_T actorID, double* vec1by3);


SSE_DATASERVICES_RTW_EXPORT_EXTERN_C
void sseSysObjGetWheelPoses(uint64_T actorID, double* mat4by4by18);


SSE_DATASERVICES_RTW_EXPORT_EXTERN_C
void sseSysObjSetWheelPoses(uint64_T actorID, double* mat4by4by18);


SSE_DATASERVICES_RTW_EXPORT_EXTERN_C
size_t sseSysObjGetNumActions(uint64_T actorID, const char* actionName);


SSE_DATASERVICES_RTW_EXPORT_EXTERN_C
void sseSysObjGetActorAction(uint64_T thisActorID,
                             const char* actionName,
                             size_t idx,
                             char* actionID,
                             uint64_T* actorID,
                             int_T* phaseInterval,
                             int_T* actionType);


SSE_DATASERVICES_RTW_EXPORT_EXTERN_C
void sseSysObjGetSpeedTarget(uint64_T actorID,
                             size_t idx,
                             double* speedValue,
                             int_T* speedComparison,
                             uint64_T* refActorID,
                             int_T* refSamplingMode);


SSE_DATASERVICES_RTW_EXPORT_EXTERN_C
void sseSysObjGetTransitionDynamics(uint64_T actorID,
                                    const char* actionName,
                                    size_t idx,
                                    int_T* dimension,
                                    int_T* shape,
                                    double* value);


SSE_DATASERVICES_RTW_EXPORT_EXTERN_C
void sseSysObjGetPathTarget(uint64_T actorID,
                            size_t idx,
                            double* pathArray,
                            uint32_T* numPoints,
                            boolean_T* hasTiming);


SSE_DATASERVICES_RTW_EXPORT_EXTERN_C
void sseSysObjGetPathPointTiming(uint64_T actorID,
                                 size_t idx,
                                 size_t pointIdx,
                                 double* time,
                                 double* speed,
                                 double* waitTime);


SSE_DATASERVICES_RTW_EXPORT_EXTERN_C
void sseSysObjGetLaneChangeTarget(uint64_T actorID,
                                  size_t idx,
                                  int_T* laneValue,
                                  int_T* laneComparison,
                                  uint64_T* refActorID);


SSE_DATASERVICES_RTW_EXPORT_EXTERN_C
void sseSysObjGetLaterlOffsetTarget(uint64_T actorID, size_t idx, double* offsetValue);


SSE_DATASERVICES_RTW_EXPORT_EXTERN_C
void sseSysObjGetDistanceTarget(uint64_T actorID,
                                size_t idx,
                                int_T* distanceDimensionType,
                                int_T* distanceType,
                                double* value,
                                int_T* actorCoordinateSystemType,
                                int_T* positionComparison,
                                int_T* measureFrom,
                                int_T* distanceComparison,
                                uint64_T* refActorID);


SSE_DATASERVICES_RTW_EXPORT_EXTERN_C
void sseSysObjGetSamplingModeAndDynamicConstraints(uint64_T actorID,
                                                   size_t idx,
                                                   int_T* referenceSamplingMode,
                                                   boolean_T* unlimitedDynamics,
                                                   double* maxSpeed,
                                                   double* maxAcceleration,
                                                   double* maxDeceleration);


SSE_DATASERVICES_RTW_EXPORT_EXTERN_C
void sseSysObjGetChangeParameter(uint64_T actorID,
                                 size_t idx,
                                 char* name,
                                 char* val,
                                 int_T* paramType);


SSE_DATASERVICES_RTW_EXPORT_EXTERN_C
size_t sseSysObjGetNumUserDefinedActions(uint64_T actorID, const char* udaName);


SSE_DATASERVICES_RTW_EXPORT_EXTERN_C
void sseSysObjGetUserDefinedActorAction(uint64_T actorID,
                                        size_t idx,
                                        const char* udaName,
                                        char* oActionID,
                                        uint64_T* oActorID,
                                        int_T* oPhaseInterval,
                                        int_T* oActionType);


SSE_DATASERVICES_RTW_EXPORT_EXTERN_C
void sseSysObjGetUDAField(uint64_T actorID,
                          size_t idx,
                          const char* udaName,
                          const char* fieldName,
                          size_t size,
                          void* buf);


SSE_DATASERVICES_RTW_EXPORT_EXTERN_C
void sseSysObjSendActionComplete(uint64_T actorID, const char* actionID);


SSE_DATASERVICES_RTW_EXPORT_EXTERN_C
size_t sseSysObjGetNumUserDefinedEvents(uint64_T actorID, const char* udeName);


/* Pull data from localRepo to generated code*/
SSE_DATASERVICES_RTW_EXPORT_EXTERN_C
void sseSysObjGetUDEField(uint64_T actorID,
                          size_t idx,
                          const char* udeName,
                          const char* fieldName,
                          size_t size,
                          void* buf);


SSE_DATASERVICES_RTW_EXPORT_EXTERN_C
void sseSysObjConstructUserDefinedEvent(uint64_T actorID, const char* udeName);


SSE_DATASERVICES_RTW_EXPORT_EXTERN_C
void sseSysObjSendUserDefinedEvent(uint64_T actorID, const char* udeName);


/* Push data from generated code to localRepo */
SSE_DATASERVICES_RTW_EXPORT_EXTERN_C
void sseSysObjSetUDEField(uint64_T actorID,
                          const char* udeName,
                          const char* fieldName,
                          FieldDataType type,
                          const void* buf);

SSE_DATASERVICES_RTW_EXPORT_EXTERN_C
void sseSysObjSetUDEString(uint64_T actorID,
                           const char* udeName,
                           const char* fieldName,
                           size_t strlen,
                           const char* buf);


SSE_DATASERVICES_RTW_EXPORT_EXTERN_C
void sseSysObjGetLaneLocation(uint64_T actorID,
                              bool* isOnLane,
                              char* laneID,
                              double* pos,
                              double* angle);


SSE_DATASERVICES_RTW_EXPORT_EXTERN_C
void sseSysObjGetActorSpecName(uint64_T actorID, char* buf);


SSE_DATASERVICES_RTW_EXPORT_EXTERN_C
void sseSysObjGetActorSpecBoundingBoxMin(uint64_T actorID, double* x, double* y, double* z);


SSE_DATASERVICES_RTW_EXPORT_EXTERN_C
void sseSysObjGetActorSpecBoundingBoxMax(uint64_T actorID, double* x, double* y, double* z);


SSE_DATASERVICES_RTW_EXPORT_EXTERN_C
void sseSysObjGetActorSpecPaintColor(uint64_T actorID,
                                     uint32_T* r,
                                     uint32_T* g,
                                     uint32_T* b,
                                     uint32_T* a);


SSE_DATASERVICES_RTW_EXPORT_EXTERN_C
uint32_T sseSysObjGetActorSpecNumWheels(uint64_T actorID);


SSE_DATASERVICES_RTW_EXPORT_EXTERN_C
void sseSysObjGetActorWheelSpec(uint64_T actorID,
                                size_t idx,
                                uint32_T* axleIndex,
                                double* offsetX,
                                double* offsetY,
                                double* offsetZ,
                                double* radius);

SSE_DATASERVICES_RTW_EXPORT_EXTERN_C
void sseSysObjGetActorType(uint64_T actorID, uint8_T* actorType);

SSE_DATASERVICES_RTW_EXPORT_EXTERN_C
uint32_T sseSysObjGetTrafficSignalRuntimeNumTurns(uint64_T actorID);

SSE_DATASERVICES_RTW_EXPORT_EXTERN_C
uint32_T sseSysObjGetTrafficSignalRuntimeNumBulbs(uint64_T actorID);

SSE_DATASERVICES_RTW_EXPORT_EXTERN_C
void sseSysObjGetTrafficSignalRuntime(uint64_T actorID,
                                      uint8_T* signalID,
                                      uint8_T* controllerID,
                                      uint32_T* configurationIndex);

SSE_DATASERVICES_RTW_EXPORT_EXTERN_C
void sseSysObjGetBulbConfiguration(uint64_T actorID,
                                   size_t idx,
                                   uint8_T* bulbName,
                                   int_T* bulbState);

SSE_DATASERVICES_RTW_EXPORT_EXTERN_C
void sseSysObjGetTimeLeftTurnConfiguration(uint64_T actorID,
                                           size_t idx,
                                           double* timeLeft,
                                           int_T* turnType,
                                           int_T* configurationType);

SSE_DATASERVICES_RTW_EXPORT_EXTERN_C
void sseSysObjGetTrafficSignalControllerRuntime(uint64_T actorID,
                                                uint8_T* controllerID,
                                                uint8_T* phaseName,
                                                uint32_T* phaseNumber,
                                                uint8_T* intervalName,
                                                int_T* intervalType,
                                                double* phaseDuration,
                                                double* phaseTimeLeft,
                                                double* cycleDuration,
                                                double* cycleTimeLeft,
                                                int_T* controllerStatus);

SSE_DATASERVICES_RTW_EXPORT_EXTERN_C
uint32_T sseSysObjGetTrafficSignalControllerSpecNumPhases(uint64_T actorID);

SSE_DATASERVICES_RTW_EXPORT_EXTERN_C
uint32_T sseSysObjGetTrafficSignalControllerSpecNumIntervals(uint64_T actorID, size_t phaseIdx);

SSE_DATASERVICES_RTW_EXPORT_EXTERN_C
void sseSysObjGetTrafficSignalControllerSpec(uint64_T actorID,
                                             size_t phaseIdx,
                                             size_t intervalIdx,
                                             uint8_T* controllerID,
                                             uint8_T* phaseName,
                                             uint8_T* intervalName,
                                             int_T* intervalType,
                                             double* intervalTime);

SSE_DATASERVICES_RTW_EXPORT_EXTERN_C
void sseSysObjGetTrafficSignalSpecNumValues(uint64_T actorID,
                                            uint32_T* numPhases,
                                            uint32_T* numSupportedTurns,
                                            uint32_T* numBulbConfiguration,
                                            uint32_T* numTurnConfiguration);

SSE_DATASERVICES_RTW_EXPORT_EXTERN_C
uint32_T sseSysObjGetTrafficSignalSpecNumIntervals(uint64_T actorID, size_t phaseIdx);

SSE_DATASERVICES_RTW_EXPORT_EXTERN_C
void sseSysObjGetTrafficSignalSpecConfigurationIndex(uint64_T actorID,
                                                     size_t phaseIdx,
                                                     size_t intervalIdx,
                                                     int_T* configurationIdx);

SSE_DATASERVICES_RTW_EXPORT_EXTERN_C
void sseSysObjGetTrafficSignalSpec(uint64_T actorID,
                                   uint8_T* signalID,
                                   uint8_T* controllerID,
                                   double* signalPosition,
                                   int_T* signalStatus);

SSE_DATASERVICES_RTW_EXPORT_EXTERN_C
int_T sseSysObjGetTrafficSignalSpecSupportedTurnTypes(uint64_T actorID, size_t idx);

SSE_DATASERVICES_RTW_EXPORT_EXTERN_C
uint32_T sseSysObjGetTrafficSignalSpecNumTurns(uint64_T actorID, size_t idx);

SSE_DATASERVICES_RTW_EXPORT_EXTERN_C
uint32_T sseSysObjGetTrafficSignalSpecNumBulbs(uint64_T actorID, size_t idx);

SSE_DATASERVICES_RTW_EXPORT_EXTERN_C
void sseSysObjGetTrafficSignalSpecBulbConfiguration(uint64_T actorID,
                                                    size_t bulbConfigurationsIdx,
                                                    size_t bulbConfigurationIdx,
                                                    uint32_T* configurationNumber,
                                                    uint8_T* bulbName,
                                                    int_T* bulbState);

SSE_DATASERVICES_RTW_EXPORT_EXTERN_C
void sseSysObjGetTrafficSignalSpecTurnConfiguration(uint64_T actorID,
                                                    size_t turnConfigurationsIdx,
                                                    size_t turnConfigurationIdx,
                                                    uint32_T* configurationNumber,
                                                    int_T* turnType,
                                                    int_T* configurationType);
/*---------------------- RRScenario Block Functions -------------------------*/

#if defined(IS_RAPID_ACCEL) || defined(IS_RSIM)
#define RAPID_AGENT
#elif defined(RT) || defined(USE_RTMODEL)
#define SLIM_AGENT
SSE_DATASERVICES_RTW_EXPORT_EXTERN_C uint64_T GET_ACTOR_ID();
#endif

SSE_DATASERVICES_RTW_EXPORT_EXTERN_C
void sseLoadActorIDRAPID(const char* pkgName, const char* modelName);

SSE_DATASERVICES_RTW_EXPORT_EXTERN_C
void sseLoadActorIDGRT(uint64_T actorID, const char* pkgName, const char* modelName);

SSE_DATASERVICES_RTW_EXPORT_EXTERN_C
void sseLoadSensorIDRAPID(char* sensorID);

SSE_DATASERVICES_RTW_EXPORT_EXTERN_C
void sseLoadSensorIDGRT(uint64_T actorID, char* sensorID);

SSE_DATASERVICES_RTW_EXPORT_EXTERN_C
void sseLoadLaneBoundaryOutputOptionRAPID(char* outputOption);

SSE_DATASERVICES_RTW_EXPORT_EXTERN_C
void sseLoadLaneBoundaryOutputOptionGRT(uint64_T actorID, char* outputOption);

SSE_DATASERVICES_RTW_EXPORT_EXTERN_C
void sseLoadTargetPosesForHostRAPID(bool isTargetPosesForHost);

SSE_DATASERVICES_RTW_EXPORT_EXTERN_C
void sseLoadTargetPosesForHostGRT(uint64_T actorID, bool isTargetPosesForHost);

SSE_DATASERVICES_RTW_EXPORT_EXTERN_C
void sseRegisterReaderBusTypeRAPID(char* busType, char* query);

SSE_DATASERVICES_RTW_EXPORT_EXTERN_C
void sseRegisterWriterBusTypeRAPID(char* busType);

SSE_DATASERVICES_RTW_EXPORT_EXTERN_C
bool sseRegisterModelRAPID();

SSE_DATASERVICES_RTW_EXPORT_EXTERN_C
bool sseRegisterModelGRT();

SSE_DATASERVICES_RTW_EXPORT_EXTERN_C
void sseResetStepRAPID();

SSE_DATASERVICES_RTW_EXPORT_EXTERN_C
void sseResetStepGRT();

SSE_DATASERVICES_RTW_EXPORT_EXTERN_C
bool sseSendMessageRAPID(void* msgData, char* msgType);

SSE_DATASERVICES_RTW_EXPORT_EXTERN_C
bool sseSendMessageGRT(uint64_T actorID, void* msgData, char* msgType);

SSE_DATASERVICES_RTW_EXPORT_EXTERN_C
bool sseReadMessageRAPID(char* msgType, char* query, void* msgData);

SSE_DATASERVICES_RTW_EXPORT_EXTERN_C
bool sseReadMessageGRT(uint64_T actorID,
                       char* sensorInfo,
                       const char* msgType,
                       const char* queryName,
                       void* msgData);

SSE_DATASERVICES_RTW_EXPORT_EXTERN_C
void sseRegisterEventGRT(uint64_T actorID, const char* eventName, const char* eventBusName);

SSE_DATASERVICES_RTW_EXPORT_EXTERN_C
void sseRegisterActionGRT(uint64_T actorID, const char* actionName, const char* actionBusName);

SSE_DATASERVICES_RTW_EXPORT_EXTERN_C
void sseResetActorGRT();

SSE_DATASERVICES_RTW_EXPORT_EXTERN_C
uint64_T sseGetActorIDRAPID();

SSE_DATASERVICES_RTW_EXPORT_EXTERN_C
void sseLoadActorID(const char* pkgName, const char* modelName);
#if defined(RAPID_AGENT)
#define sseLoadActorID(pkgName, modelName) sseLoadActorIDRAPID(pkgName, modelName)
#elif defined(SLIM_AGENT)
#define sseLoadActorID(pkgName, modelName) sseLoadActorIDGRT(GET_ACTOR_ID(), pkgName, modelName)
#endif

SSE_DATASERVICES_RTW_EXPORT_EXTERN_C
void sseLoadSensorID(char* sensorID);
#if defined(RAPID_AGENT)
#define sseLoadSensorID(sensorID) sseLoadSensorIDRAPID(sensorID)
#elif defined(SLIM_AGENT)
#define sseLoadSensorID(sensorID) sseLoadSensorIDGRT(GET_ACTOR_ID(), sensorID)
#endif

SSE_DATASERVICES_RTW_EXPORT_EXTERN_C
void sseLoadLaneBoundaryOutputOption(char* outputOption);
#if defined(RAPID_AGENT)
#define sseLoadLaneBoundaryOutputOption(outputOption) \
    sseLoadLaneBoundaryOutputOptionRAPID(outputOption)
#elif defined(SLIM_AGENT)
#define sseLoadLaneBoundaryOutputOption(outputOption) \
    sseLoadLaneBoundaryOutputOptionGRT(GET_ACTOR_ID(), outputOption)
#endif

SSE_DATASERVICES_RTW_EXPORT_EXTERN_C
void sseLoadTargetPosesForHost(bool isTargetPosesForHost);
#if defined(RAPID_AGENT)
#define sseLoadTargetPosesForHost(isTargetPosesForHost) \
    sseLoadTargetPosesForHostRAPID(isTargetPosesForHost)
#elif defined(SLIM_AGENT)
#define sseLoadTargetPosesForHost(isTargetPosesForHost) \
    sseLoadTargetPosesForHostGRT(GET_ACTOR_ID(), isTargetPosesForHost)
#endif

SSE_DATASERVICES_RTW_EXPORT_EXTERN_C
void sseRegisterReaderBusType(char* busType, char* query);
#if defined(RAPID_AGENT)
#define sseRegisterReaderBusType(busType, query) sseRegisterReaderBusTypeRAPID(busType, query)
#elif defined(SLIM_AGENT)
#define sseRegisterReaderBusType(busType, query)
#endif

SSE_DATASERVICES_RTW_EXPORT_EXTERN_C
void sseRegisterWriterBusType(char* busType);
#if defined(RAPID_AGENT)
#define sseRegisterWriterBusType(busType) sseRegisterWriterBusTypeRAPID(busType)
#elif defined(SLIM_AGENT)
#define sseRegisterWriterBusType(busType)
#endif

SSE_DATASERVICES_RTW_EXPORT_EXTERN_C
bool sseRegisterModel();
#if defined(RAPID_AGENT)
#define sseRegisterModel() sseRegisterModelRAPID()
#elif defined(SLIM_AGENT)
#define sseRegisterModel() sseRegisterModelGRT()
#endif

SSE_DATASERVICES_RTW_EXPORT_EXTERN_C
void sseResetStep();
#if defined(RAPID_AGENT)
#define sseResetStep() sseResetStepRAPID();
#elif defined(SLIM_AGENT)
#define sseResetStep() sseResetStepGRT();
#endif

SSE_DATASERVICES_RTW_EXPORT_EXTERN_C
void sseRegisterEvent(const char* eventName, const char* eventBusName);
#if defined(RAPID_AGENT)
#define sseRegisterEvent(eventName, eventBusName)
#elif defined(SLIM_AGENT)
#define sseRegisterEvent(eventName, eventBusName) \
    sseRegisterEventGRT(GET_ACTOR_ID(), eventName, eventBusName)
#endif

SSE_DATASERVICES_RTW_EXPORT_EXTERN_C
void sseRegisterAction(const char* actionName, const char* actionBusName);
#if defined(RAPID_AGENT)
#define sseRegisterAction(actionName, actionBusName)
#elif defined(SLIM_AGENT)
#define sseRegisterAction(actionName, actionBusName) \
    sseRegisterActionGRT(GET_ACTOR_ID(), actionName, actionBusName)
#endif

SSE_DATASERVICES_RTW_EXPORT_EXTERN_C
uint64_T sseGetActorID();
#if defined(RAPID_AGENT)
#define sseGetActorID() sseGetActorIDRAPID()
#elif defined(SLIM_AGENT)
#define sseGetActorID() GET_ACTOR_ID()
#endif

SSE_DATASERVICES_RTW_EXPORT_EXTERN_C
bool sseSendMessage(void* msgData, char* msgType);
#if defined(RAPID_AGENT)
#define sseSendMessage(msgData, msgType) sseSendMessageRAPID(msgData, msgType)
#elif defined(SLIM_AGENT)
#define sseSendMessage(msgData, msgType) sseSendMessageGRT(GET_ACTOR_ID(), msgData, msgType)
#endif

SSE_DATASERVICES_RTW_EXPORT_EXTERN_C
bool sseReadMessage(char* msgType, char* queryType, void* msgData, char* sensorInfo);
#if defined(RAPID_AGENT)
#define sseReadMessage(msgType, queryType, msgData, sensorInfo) \
    sseReadMessageRAPID(msgType, queryType, msgData)
#elif defined(SLIM_AGENT)
#define sseReadMessage(msgType, queryType, msgData, sensorInfo) \
    sseReadMessageGRT(GET_ACTOR_ID(), sensorInfo, msgType, queryType, msgData)
#endif

SSE_DATASERVICES_RTW_EXPORT_EXTERN_C
void sseResetActor();
#if defined(RAPID_AGENT)
#define sseResetActor()
#elif defined(SLIM_AGENT)
#define sseResetActor() sseResetActorGRT()
#endif

#endif
