/* Copyright 2017-2022 The MathWorks, Inc. */
/*
 * @file: RTWCG_FMU3_target.h
 *
 * @brief fmustruct
 *
 * Abstract:
 *      FMI 2.0 data types, function signatures and FMU3_CS/ME_RTWCG(struct)
 *
 *      FMU3_CS/ME_RTWCG is a data struct to store FMU info, handle all
 *      dynamic function calls to FMU lib
 *
 * =============================================================================
 */
#ifndef RTWCG_FMU3_GUARD
#define RTWCG_FMU3_GUARD

#ifdef __cplusplus
extern "C"{
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <stdarg.h>
#ifndef __bool_true_false_are_defined
#undef true /* may be defined in rtwtypes.h */
#undef false /* may be defined in rtwtypes.h */
#include <stdbool.h>
#endif
#include "fmi3Functions.h"

#include "FMUCG_Target.h"
#if FMU_CG_TARGET < FMUCG_STANDALONE_TARGETS
#include <simstruc.h>
#include <slsv_diagnostic_codegen_c_api.h>
/*raccel_utils.h is needed by win plateform to mute warning warning C4013*/
#include "rapid/raccel_utils.h"
#if FMU_CG_TARGET == FMUCG_PROTECTED_MODEL
#include "RTWCG_FMU_util.h"
#elif FMU_CG_TARGET == FMUCG_MCC
#include "RTWCG_FMU_util.h"
#endif
#else
#include "rtwtypes.h"
#include "RTWCG_FMU_util.h"
#endif

#ifdef _WIN32
#include "windows.h"
#define FMUHANDLE                      HMODULE
#define LOAD_LIBRARY(LIBRARY_LOC)      loadLibraryUTF8toUTF16(LIBRARY_LOC)
#define LOAD_FUNCTION                  GetProcAddress
#define CLOSE_LIBRARY                  FreeLibrary

#else
#include <dlfcn.h>
#define FMUHANDLE                      void*
#ifdef RTLD_DEEPBIND
#define LOAD_LIBRARY(LIBRARY_LOC)      dlopen(LIBRARY_LOC, RTLD_NOW | RTLD_LOCAL | RTLD_DEEPBIND)
#else
#define LOAD_LIBRARY(LIBRARY_LOC)      dlopen(LIBRARY_LOC, RTLD_NOW | RTLD_LOCAL)
#endif
#define LOAD_FUNCTION                  dlsym
#define CLOSE_LIBRARY                  dlclose
#endif

#define FULL_FCN_NAME_MAX_LEN 256
#define FCN_NAME_MAX_LEN      28

#ifndef getgblSetParamPktReceived_GUARD
#define getgblSetParamPktReceived_GUARD
#define getgblSetParamPktReceived(ptr)       {  \
    extern boolean_T   gblSetParamPktReceived;  \
    *ptr = gblSetParamPktReceived;              \
    }
#endif

#ifndef isSimTargetSimulating_GUARD
#define isSimTargetSimulating_GUARD
#define isSimTargetSimulating(S)                \
    ssGetSimMode((SimStruct*)(S))==SS_SIMMODE_NORMAL || ssRTWGenIsAccelerator((SimStruct*)(S))
#endif

#ifndef ssGetMdlSfuncS_GUARD
#define ssGetMdlSfuncS_GUARD
#define ssGetMdlSfuncS(S)                \
    (S)->_mdlRefSfcnS
#endif

typedef fmi3Status (*_fmi3_default_status_fcn_type) (fmi3Instance, ...);
typedef const char* (*_fmi3_default_string_fcn_type) (void);
typedef void* (*_fmi3_default_instance_fcn_type) (const char*, ...);
typedef void (*_fmi3_default_void_fcn_type) (void*);

struct FMU3_loggerEnv {
    bool isLoggingOn;
    char* fmuname;
    #if FMU_CG_TARGET < FMUCG_STANDALONE_TARGETS
    SimStruct *ssPtr;
    #endif
};

struct FMU3_CSME_RTWCG {
    /* isLoggingOn, fmuname, ssPtr are defined on top of the struct so that it aligns FMU3_CSME_RTWCG and FMU3_loggerEnv*/
    bool isLoggingOn;
    char* fmuname;
    #if FMU_CG_TARGET < FMUCG_STANDALONE_TARGETS
    SimStruct *ssPtr;
    #endif
    /*common functions*/
        /* Utility */
        fmi3GetVersionTYPE* getVersion;
        fmi3SetDebugLoggingTYPE* setDebugLogging;
        /* Mode switch */
        fmi3EnterConfigurationModeTYPE* enterConfigurationMode;
        fmi3ExitConfigurationModeTYPE* exitConfigurationMode;
        fmi3EnterInitializationModeTYPE* enterInitializationMode;
        fmi3ExitInitializationModeTYPE*  exitInitializationMode;
        fmi3TerminateTYPE* terminate;
        fmi3ResetTYPE* reset;
        /* Startup/cleanup */
        fmi3FreeInstanceTYPE* freeInstance;
        /* data marshalling */
        fmi3GetFloat32TYPE* getFloat32;
        fmi3GetFloat64TYPE* getFloat64;
        fmi3GetInt8TYPE* getInt8;
        fmi3GetUInt8TYPE* getUInt8;
        fmi3GetInt16TYPE* getInt16;
        fmi3GetUInt16TYPE* getUInt16;
        fmi3GetInt32TYPE* getInt32;
        fmi3GetUInt32TYPE* getUInt32;
        fmi3GetInt64TYPE* getInt64;
        fmi3GetUInt64TYPE* getUInt64;
        fmi3GetBooleanTYPE* getBoolean;
        fmi3GetStringTYPE* getString;
        fmi3GetBinaryTYPE* getBinary;
        fmi3GetClockTYPE* getClock;
        fmi3SetFloat32TYPE* setFloat32;
        fmi3SetFloat64TYPE* setFloat64;
        fmi3SetInt8TYPE* setInt8;
        fmi3SetUInt8TYPE* setUInt8;
        fmi3SetInt16TYPE* setInt16;
        fmi3SetUInt16TYPE* setUInt16;
        fmi3SetInt32TYPE* setInt32;
        fmi3SetUInt32TYPE* setUInt32;
        fmi3SetInt64TYPE* setInt64;
        fmi3SetUInt64TYPE* setUInt64;
        fmi3SetBooleanTYPE* setBoolean;
        fmi3SetStringTYPE* setString;
        fmi3SetBinaryTYPE* setBinary;
        fmi3SetClockTYPE* setClock;
        /* Event */
        fmi3SetIntervalDecimalTYPE* setIntervalDecimal;
        fmi3SetIntervalFractionTYPE* setIntervalFraction;
        fmi3GetIntervalDecimalTYPE* getIntervalDecimal;
        fmi3GetIntervalFractionTYPE* getIntervalFraction;
        fmi3GetShiftDecimalTYPE* getShiftDecimal;
        fmi3GetShiftFractionTYPE* getShiftFraction;
        fmi3SetShiftDecimalTYPE* setShiftDecimal;
        fmi3SetShiftFractionTYPE* setShiftFraction;
        /* Save/restore */
        fmi3GetFMUStateTYPE* getFMUState;
        fmi3SetFMUStateTYPE* setFMUState;
        fmi3FreeFMUStateTYPE* freeFMUState;
        fmi3SerializedFMUStateSizeTYPE* serializedFMUStateSize;
        fmi3SerializeFMUStateTYPE* serializeFMUState;
        fmi3DeserializeFMUStateTYPE* deserializeFMUState;
        /* Model structure */
        fmi3GetNumberOfVariableDependenciesTYPE* getNumberOfVariableDependencies;
        fmi3GetVariableDependenciesTYPE* getVariableDependencies;
        /* Jacobian */
        fmi3GetDirectionalDerivativeTYPE* getDirectionalDerivative;
        fmi3GetAdjointDerivativeTYPE* getAdjointDerivative;
        /* ME/CS semantics */
        fmi3EnterEventModeTYPE* enterEventMode;
        fmi3UpdateDiscreteStatesTYPE* updateDiscreteStates;
        /* Intermediate update */
        fmi3EvaluateDiscreteStatesTYPE* evaluateDiscreteStates;


    /*fmi me functions*/
        /* Startup/cleanup */
        fmi3InstantiateModelExchangeTYPE* instantiateModelExchange;
        /* ME semantics */
        fmi3GetNumberOfContinuousStatesTYPE* getNumberOfContinuousStates;
        fmi3GetNumberOfEventIndicatorsTYPE* getNumberOfEventIndicators;
        fmi3EnterContinuousTimeModeTYPE* enterContinuousTimeMode;
        fmi3GetContinuousStateDerivativesTYPE* getContinuousStateDerivatives;
        fmi3GetContinuousStatesTYPE* getContinuousStates;
        fmi3GetNominalsOfContinuousStatesTYPE* getNominalsOfContinuousStates;
        fmi3GetEventIndicatorsTYPE* getEventIndicators;
        fmi3SetTimeTYPE* setTime;
        fmi3SetContinuousStatesTYPE* setContinuousStates;
        fmi3CompletedIntegratorStepTYPE* completedIntegratorStep;

    /*fmi cs functions*/
        /* Startup/cleanup */
        fmi3InstantiateCoSimulationTYPE* instantiateCoSimulation;
        /* CS semantics */
        fmi3EnterStepModeTYPE* enterStepMode;
        fmi3DoStepTYPE* doStep;
        /* Compensation */
        fmi3GetOutputDerivativesTYPE* getOutputDerivatives;

    char* dllfile;
    FMUHANDLE Handle;
    fmi3Instance mFMIComp;
    fmi3InstanceEnvironment instanceEnvironment;
    fmi3Status FMUErrorStatus;
    fmi3Boolean newDiscreteStatesNeeded;
    fmi3Boolean terminateSimulation;
    fmi3Boolean nominalsOfContinuousStatesChanged;
    fmi3Boolean valuesOfContinuousStatesChanged;
    fmi3Boolean nextEventTimeDefined;
    fmi3Float64 nextEventTime;
    bool modelInitialized;
    fmi3LogMessageCallback logMessage;
    /*two int arrays for maping enum param original value to actual value*/
    int* paramIdxToOffset;
    int* enumValueList;
};

/* RTWCG entry points for FMU3 */
void* FMU3_fmuInitializeCS(const char* lib,
                           fmi3String instanceName,
                           fmi3String instantiationToken,
                           fmi3String resourcePath,
                           void* ssPtr,
                           int loadFMUStateFcn,
                           int loadSerializationFcn,
                           int loadDirectionDerivativeFcn,
                           fmi3Boolean visible,
                           fmi3Boolean loggingOn,
                           fmi3InstanceEnvironment instanceEnvironment,
                           fmi3LogMessageCallback logMessage,
                           fmi3Boolean eventModeUsed,
                           fmi3Boolean earlyReturnAllowed,
                           const fmi3ValueReference requiredIntermediateVariables[],
                           size_t nRequiredIntermediateVariables,
                           fmi3IntermediateUpdateCallback intermediateUpdate);

fmi3Boolean FMU3_doStep(void **fmuv,
                        double currentCommunicationPoint,
                        double communicationStepSize,
                        fmi3Boolean noSetFMUStatePriorToCurrentPoint,
                        void* eventHandlingNeeded);

fmi3Boolean FMU3_terminate(void **fmuv);

fmi3Boolean FMU3_enterInitializationMode(void **fmuv);

fmi3Boolean FMU3_exitInitializationMode(void **fmuv);

////////////////////////////////////////////////////////////////////
fmi3Boolean FMU3_setFloat64Ptr(void **fmuv,
                            const fmi3ValueReference dvr,
                            size_t nvr,
                            const fmi3Float64 value[],
                            size_t nVal);
fmi3Boolean FMU3_setFloat32Ptr(void **fmuv,
                           const fmi3ValueReference dvr,
                           size_t nvr,
                           const fmi3Float32 value[],
                           size_t nVal);
fmi3Boolean FMU3_setInt8Ptr(void **fmuv,
                           const fmi3ValueReference dvr,
                           size_t nvr,
                           const fmi3Int8 value[],
                           size_t nVal);
fmi3Boolean FMU3_setUInt8Ptr(void **fmuv,
                           const fmi3ValueReference dvr,
                           size_t nvr,
                           const fmi3UInt8 value[],
                           size_t nVal);
fmi3Boolean FMU3_setInt16Ptr(void **fmuv,
                           const fmi3ValueReference dvr,
                           size_t nvr,
                           const fmi3Int16 value[],
                           size_t nVal);
fmi3Boolean FMU3_setUInt16Ptr(void **fmuv,
                           const fmi3ValueReference dvr,
                           size_t nvr,
                           const fmi3UInt16 value[],
                           size_t nVal);
fmi3Boolean FMU3_setInt32Ptr(void **fmuv,
                           const fmi3ValueReference dvr,
                           size_t nvr,
                           const fmi3Int32 value[],
                           size_t nVal);
fmi3Boolean FMU3_setUInt32Ptr(void **fmuv,
                           const fmi3ValueReference dvr,
                           size_t nvr,
                           const fmi3UInt32 value[],
                           size_t nVal);
fmi3Boolean FMU3_setInt64EnumPtr(void **fmuv,
                           const fmi3ValueReference dvr,
                           size_t nvr,
                           const void* value,
                           size_t nVal);
fmi3Boolean FMU3_setUInt64Ptr(void **fmuv,
                           const fmi3ValueReference dvr,
                           size_t nvr,
                           const void* value,
                           size_t nVal);
fmi3Boolean FMU3_setBooleanPtr(void **fmuv,
                           const fmi3ValueReference dvr,
                           size_t nvr,
                           const fmi3UInt8 value[],
                           size_t nVal);
fmi3Boolean FMU3_setStringPtr(void **fmuv,
                          const fmi3ValueReference dvr,
                          size_t nvr,
                          const fmi3String value[]);
////////////////////////////////////////////////////////////////////

fmi3Boolean FMU3_setFloat64(void **fmuv,
                        const fmi3ValueReference vr[],
                        size_t nvr,
                        const fmi3Float64 value[],
                        size_t nVal);
fmi3Boolean FMU3_setFloat32(void **fmuv,
                        const fmi3ValueReference vr[],
                        size_t nvr,
                        const fmi3Float32 value[],
                        size_t nVal);
fmi3Boolean FMU3_setInt8(void **fmuv,
                        const fmi3ValueReference vr[],
                        size_t nvr,
                        const fmi3Int8 value[],
                        size_t nVal);
fmi3Boolean FMU3_setUInt8(void **fmuv,
                        const fmi3ValueReference vr[],
                        size_t nvr,
                        const fmi3UInt8 value[],
                        size_t nVal);
fmi3Boolean FMU3_setInt16(void **fmuv,
                        const fmi3ValueReference vr[],
                        size_t nvr,
                        const fmi3Int16 value[],
                        size_t nVal);
fmi3Boolean FMU3_setUInt16(void **fmuv,
                        const fmi3ValueReference vr[],
                        size_t nvr,
                        const fmi3UInt16 value[],
                        size_t nVal);
fmi3Boolean FMU3_setInt32(void **fmuv,
                        const fmi3ValueReference vr[],
                        size_t nvr,
                        const fmi3Int32 value[],
                        size_t nVal);
fmi3Boolean FMU3_setUInt32(void **fmuv,
                        const fmi3ValueReference vr[],
                        size_t nvr,
                        const fmi3UInt32 value[],
                        size_t nVal);
fmi3Boolean FMU3_setInt64Enum(void **fmuv,
                        const fmi3ValueReference vr[],
                        size_t nvr,
                        const void* value,
                        size_t nVal);
fmi3Boolean FMU3_setUInt64(void **fmuv,
                        const fmi3ValueReference vr[],
                        size_t nvr,
                        const void* value,
                        size_t nVal);
fmi3Boolean FMU3_setBoolean(void **fmuv,
                           const fmi3ValueReference vr[],
                           size_t nvr,
                           const fmi3UInt8 value[],
                           size_t nVal);
fmi3Boolean FMU3_setString(void **fmuv,
                          const fmi3ValueReference vr[],
                          size_t nvr,
                          void* value[]);
fmi3Boolean FMU3_getFloat64(void **fmuv,
                        const fmi3ValueReference vr[],
                        size_t nvr,
                        fmi3Float64 value[],
                        size_t nVal);
fmi3Boolean FMU3_getFloat32(void **fmuv,
                        const fmi3ValueReference vr[],
                        size_t nvr,
                        fmi3Float32 value[],
                        size_t nVal);
fmi3Boolean FMU3_getInt8(void **fmuv,
                        const fmi3ValueReference vr[],
                        size_t nvr,
                        fmi3Int8 value[],
                        size_t nVal);
fmi3Boolean FMU3_getUInt8(void **fmuv,
                        const fmi3ValueReference vr[],
                        size_t nvr,
                        fmi3UInt8 value[],
                        size_t nVal);
fmi3Boolean FMU3_getInt16(void **fmuv,
                        const fmi3ValueReference vr[],
                        size_t nvr,
                        fmi3Int16 value[],
                        size_t nVal);
fmi3Boolean FMU3_getUInt16(void **fmuv,
                        const fmi3ValueReference vr[],
                        size_t nvr,
                        fmi3UInt16 value[],
                        size_t nVal);
fmi3Boolean FMU3_getInt32(void **fmuv,
                        const fmi3ValueReference vr[],
                        size_t nvr,
                        fmi3Int32 value[],
                        size_t nVal);
fmi3Boolean FMU3_getUInt32(void **fmuv,
                        const fmi3ValueReference vr[],
                        size_t nvr,
                        fmi3UInt32 value[],
                        size_t nVal);
fmi3Boolean FMU3_getInt64Enum(void **fmuv,
                        const fmi3ValueReference vr[],
                        size_t nvr,
                        void* value,
                        size_t nVal);
fmi3Boolean FMU3_getUInt64(void **fmuv,
                        const fmi3ValueReference vr[],
                        size_t nvr,
                        void* value,
                        size_t nVal);
fmi3Boolean FMU3_getBoolean(void **fmuv,
                           const fmi3ValueReference vr[],
                           size_t nvr,
                           fmi3UInt8 value[],
                           size_t nVal);
fmi3Boolean FMU3_getString(void **fmuv,
                          const fmi3ValueReference vr[],
                          size_t nvr,
                          void* value[]);

////////////////////////////////////////////////////////////////////

void FMU3_strncpy(char* dest, void* src, int size);

char* FMU3_strndup(const char* src, int size);

void FMU3_simTerminate(void **fmuv,
                       const char* blkPath,
                       fmi3Float64 time);

void FMU3_terminateModel(void **fmuv);

void FMU3_getNextEventTime(void **fmuv,
                           fmi3Float64* nextEventTime,
                           int32_T* upcomingTimeEvent);

void * FMU3_fmuInitializeME(const char* lib,
                           fmi3String instanceName,
                           fmi3String instantiationToken,
                           fmi3String resourcePath,
                           void* ssPtr,
                           int loadFMUStateFcn,
                           int loadSerializationFcn,
                           int loadDirectionDerivativeFcn,
                           fmi3Boolean visible,
                           fmi3Boolean loggingOn,
                           fmi3InstanceEnvironment instanceEnvironment,
                           fmi3LogMessageCallback logMessage);

fmi3Boolean FMU3_eventIteration(void **fmuv,
                                const char* blkPath,
                                fmi3Float64 time);

void FMU3_valuesOfContinuousStatesChanged(void **fmuv,
                                          int* stateChanged);

fmi3Boolean FMU3_enterStepMode(void** fmuv);

fmi3Boolean FMU3_enterEventMode(void** fmuv);


fmi3Boolean FMU3_enterContinuousTimeMode(void** fmuv);

fmi3Boolean FMU3_completedIntegratorStep(void** fmuv,
                                        fmi3Boolean,
                                        uint8_t*,
                                        uint8_t*);

fmi3Boolean FMU3_setTime(void** fmuv,
                        fmi3Float64);

fmi3Boolean FMU3_setContinuousStates(void** fmuv,
                                    const fmi3Float64[],
                                    size_t);

fmi3Boolean FMU3_getDerivatives(void** fmuv,
                               fmi3Float64[],
                               size_t);

fmi3Boolean FMU3_getEventIndicators(void** fmuv,
                                   fmi3Float64[],
                                   size_t);

fmi3Boolean FMU3_getContinuousStates(void** fmuv,
                                    fmi3Float64[],
                                    size_t);

fmi3Boolean FMU3_getNominalsOfContinuousStates(void** fmuv,
                                              fmi3Float64[],
                                              size_t);

void FMU3_setFMUState(void** fmuv, void* fmustate);
void FMU3_getFMUState(void** fmuv, void** fmustate);
void FMU3_freeFMUState(void** fmuv, void** fmustate);
void FMU3_serializedFMUstateSize(void** fmuv, void* fmustate, size_t* size);
void FMU3_serializeFMUstate(void** fmuv, void* fmustate, char* byteArray);
void FMU3_deserializeFMUstate(void** fmuv, const char* byteArray, size_t size, void** fmustate);

/*helper to preprocess Enum type*/
void FMU3_CreateParamIdxToOffset(void** fmuv,
                            int array_size);

void FMU3_CreateEnumValueList(void** fmuv,
                         int array_size);

void FMU3_SetParamIdxToOffsetByIdx(void** fmuv,
                              int idx,
                              int value);

void FMU3_SetEnumValueListByIdx(void** fmuv,
                           int idx,
                           int value);

void FMU3_GetParamIdxToOffsetByIdx(void** fmuv,
                              int idx,
                              int* offset);

void FMU3_GetEnumValueByIdx(void** fmuv,
                       int idx,
                       int* val);

void FMU3_CompareInputWithWorkingVec(void *dstWorkingVec, const void* srcBlkInput, const uint32_t* dimList, const size_t nVal, const size_t datasize, bool needConversion, int32_t *is_diff_value);
void FMU3_UpdateVectorForFmu(void *dstWorkingVec, const void* srcBlkInput, const uint32_t* dimList, const size_t nVal, const size_t datasize, bool needConversion);
void FMU3_UpdateVectorForBlk(void *dstBlkOutput, const void* srcWorkingVector, const uint32_t* dimList, const size_t nVal, const size_t datasize, bool needConversion);

#ifdef __cplusplus
}
#endif

#endif
