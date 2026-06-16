/* Copyright 2017-2024 The MathWorks, Inc. */

/******************************************************************************
 *                                                                            *
 * File    : RTWCG_FMU3_target.c                                              *
 * Abstract:                                                                  *
 *      Wrapper functions to dynamic library calls of FMU 2.0                 *
 *      This file defines functions called by code generation                 *
 *      Also handle errors, and logger                                        *
 *                                                                            *
 ******************************************************************************/

#include "RTWCG_FMU3_target.h"
#include <stdlib.h>
#define FMU3_MESSAGE_SIZE 1024

#ifndef UNUSED_PARAMETER
#define UNUSED_PARAMETER(x)     (void) (x)
#endif

#if FMU_CG_TARGET == FMUCG_SLRT
#include "SLRTLoggerWrapper.hpp"
#endif
/*
  Whenever a default function is called, it means a functions is called without successful load,
  return a fmi3Error;
*/
static fmi3Status defaultfcnStatus(fmi3Instance c, ...) {
    (void)c;
    return fmi3Error;
}
static const char* defaultfcnString(void) {
    return NULL;
}
static void* defaultfcnInstance(const char* c,...) {
    (void)c;
    return NULL;
}
static void defaultfcnVoid(void* c) {
    (void)c;
    return;
}

static fmi3String formatString(fmi3String format, ...) {
    static char translatedMsg[FMU3_MESSAGE_SIZE];
    va_list args;
    va_start(args, format);
    vsnprintf(translatedMsg, FMU3_MESSAGE_SIZE - 1, format, args);
    va_end(args);
    return translatedMsg;
}

static const char* getLoadLibraryErrorMessage(void)
{
    static char message[FMU3_MESSAGE_SIZE];
#ifdef _WIN32
    unsigned long int errorCode = GetLastError();
    snprintf(message, FMU3_MESSAGE_SIZE, "Load Library Error: '%d' ", errorCode);
#else
    const char* errorMessage = dlerror();
    snprintf(message, FMU3_MESSAGE_SIZE, "Load Library Error: '%s' ", errorMessage);
#endif
    return message;
}

static void FMU3_Logger(fmi3InstanceEnvironment c,
                       fmi3Status status,
                       fmi3String category,
                       fmi3String message) {

    static const char* strStatus[] = {
        "fmi3OK", "fmi3Warning", "fmi3Discard", "fmi3Error", "fmi3Fatal"};

    struct FMU3_loggerEnv * loggerEnv = (struct FMU3_loggerEnv *)c;
    if (!loggerEnv || !loggerEnv->isLoggingOn) return;

    char translatedMsg[FMU3_MESSAGE_SIZE];
    int prefixLength = snprintf(translatedMsg, FMU3_MESSAGE_SIZE, "Log from FMU %s: [category:%s, status:%s] ",
                            loggerEnv->fmuname, strStatus[status], category);
    strncat(translatedMsg, message, FMU3_MESSAGE_SIZE-prefixLength - 1);

#if FMU_CG_TARGET < FMUCG_STANDALONE_TARGETS
    SimStruct* ssPtr = (SimStruct*)loggerEnv->ssPtr;
    void *diagnostic = CreateDiagnosticAsVoidPtr("SL_SERVICES:utils:PRINTFWRAPPER", 1,
                                           CODEGEN_SUPPORT_ARG_STRING_TYPE, translatedMsg);
    rt_ssReportDiagnosticAsInfo(ssPtr, diagnostic);
#elif FMU_CG_TARGET == FMUCG_SLRT
    /* Skip printing info logs which are shown by fmiOK status*/
    if(status != fmi3OK) {
           switch (status){
        case fmi3Error:
            SLRTLog(kError,translatedMsg);
            break;
        case fmi3Fatal:
            SLRTLog(kFatal,translatedMsg);
            break;
        case fmi3Warning:
        default:
            SLRTLog(kWarning,translatedMsg);
            break;
        }
    }
#else
    printf("%s\n", translatedMsg);
#endif
}

#ifdef _WIN32
static FMUHANDLE loadLibraryUTF8toUTF16(const char* library_loc)
{
    int convertResult = MultiByteToWideChar(CP_UTF8, 0, library_loc, (int)strlen(library_loc), NULL, 0);
    if (convertResult <= 0) {
        return NULL;
    }
    wchar_t* wLibrary_loc = (wchar_t*)calloc((convertResult + 1), sizeof(wchar_t));
    MultiByteToWideChar(CP_UTF8, 0, library_loc, (int)strlen(library_loc), wLibrary_loc, convertResult);
    FMUHANDLE handle = LoadLibraryExW(wLibrary_loc, NULL, LOAD_WITH_ALTERED_SEARCH_PATH);
    free(wLibrary_loc);
    return handle;
}
#endif

#define LoadFMUFcnDecl1(f, x)                                           \
    static x f(struct FMU3_CSME_RTWCG* fmustruct,                       \
               const char * fcnName,                                    \
               int isOptional) {                                        \
    x fcn = NULL;                                                       \
    fmi3String message = NULL;

#if FMU_CG_TARGET < FMUCG_STANDALONE_TARGETS
#define LoadFMUFcnDecl2                         \
    (void) message;
#else
#define LoadFMUFcnDecl2
#endif

#ifdef _WIN32

#define LoadFMUFcnDecl3(x)                                              \
    fcn = (x)LOAD_FUNCTION(fmustruct->Handle, fcnName);

#else

#if FMU_CG_TARGET == FMUCG_SLRT

#define LoadFMUFcnDecl3(x)                                              \
    /* creating a char array "fmuname_functionName" as the name to fetch from .so file. */ \
    char fullFcnName[FULL_FCN_NAME_MAX_LEN];                            \
    memset(fullFcnName, 0, FULL_FCN_NAME_MAX_LEN);                      \
    strncpy(fullFcnName, fmustruct->fmuname, FULL_FCN_NAME_MAX_LEN - 1); \
    strncat(fullFcnName, "_", FULL_FCN_NAME_MAX_LEN - strlen(fullFcnName) - 1); \
    strncat(fullFcnName, fcnName, FULL_FCN_NAME_MAX_LEN - strlen(fullFcnName) - 1); \
    *((void **)(&fcn)) = LOAD_FUNCTION(fmustruct->Handle, fullFcnName);

#else

#define LoadFMUFcnDecl3(x)                                              \
    *((void **)(&fcn)) = LOAD_FUNCTION(fmustruct->Handle, fcnName);

#endif
#endif

#define LoadFMUFcnDecl4                         \
    if (!isOptional && fcn == NULL) {           \
    fmustruct->FMUErrorStatus = fmi3Warning;

#if FMU_CG_TARGET < FMUCG_STANDALONE_TARGETS

#define LoadFMUFcnDecl5                                                 \
    if (fmustruct->isLoggingOn) {                                       \
        void * diagnostic = CreateDiagnosticAsVoidPtr("FMUBlock:FMU:FMULoadLibFunctionError", 2, \
                                                      CODEGEN_SUPPORT_ARG_STRING_TYPE, fcnName, \
                                                      CODEGEN_SUPPORT_ARG_STRING_TYPE, fmustruct->fmuname); \
        /*Loading failure will cause a warning, ANY CALL to default fcn will result in fmiError and stop*/ \
        rt_ssReportDiagnosticAsWarning(fmustruct->ssPtr, diagnostic);   \
    }

#else

#define LoadFMUFcnDecl5                                                 \
    message = formatString("Unable to load function '%s' for FMU '%s'.", fcnName, fmustruct->fmuname); \
    fmustruct->logMessage(fmustruct->instanceEnvironment, fmustruct->FMUErrorStatus, fcnName, message);

#endif

#define LoadFMUFcnDecl6(x, d)                                   \
    fcn = (x) d;                                                \
    }                                                           \
    return fcn;                                                 \
    }

#define LoadFMUFcnDecl(f, x, d)                                         \
    LoadFMUFcnDecl1(f, x) LoadFMUFcnDecl2 LoadFMUFcnDecl3(x) LoadFMUFcnDecl4 LoadFMUFcnDecl5 LoadFMUFcnDecl6(x, d)

LoadFMUFcnDecl(LoadFMUFcnStatus, _fmi3_default_status_fcn_type, defaultfcnStatus)
LoadFMUFcnDecl(LoadFMUFcnString, _fmi3_default_string_fcn_type, defaultfcnString)
LoadFMUFcnDecl(LoadFMUFcnInstance, _fmi3_default_instance_fcn_type, defaultfcnInstance)
LoadFMUFcnDecl(LoadFMUFcnVoid, _fmi3_default_void_fcn_type, defaultfcnVoid)

/*Load FMU 3 Common Functions*/
static void LoadFMU3CommonFunctions(struct FMU3_CSME_RTWCG* fmustruct,
                                    int loadFMUStateFcn,
                                    int loadSerializationFcn,
                                    int loadDirectionDerivativeFcn) {
        /* Utility */
        fmustruct->getVersion                   = (fmi3GetVersionTYPE*)  LoadFMUFcnString(fmustruct, "fmi3GetVersion", 0);
        fmustruct->setDebugLogging              = (fmi3SetDebugLoggingTYPE*)  LoadFMUFcnStatus(fmustruct, "fmi3SetDebugLogging", 0);
        /* Mode switch */
        fmustruct->enterConfigurationMode       = (fmi3EnterConfigurationModeTYPE*)  LoadFMUFcnStatus(fmustruct, "fmi3EnterConfigurationMode", 0);
        fmustruct->exitConfigurationMode        = (fmi3ExitConfigurationModeTYPE*)  LoadFMUFcnStatus(fmustruct, "fmi3ExitConfigurationMode", 0);
        fmustruct->enterInitializationMode      = (fmi3EnterInitializationModeTYPE*)  LoadFMUFcnStatus(fmustruct, "fmi3EnterInitializationMode", 0);
        fmustruct-> exitInitializationMode      = (fmi3ExitInitializationModeTYPE*)  LoadFMUFcnStatus(fmustruct, "fmi3ExitInitializationMode", 0);
        fmustruct->terminate                    = (fmi3TerminateTYPE*)  LoadFMUFcnStatus(fmustruct, "fmi3Terminate", 0);
        fmustruct->reset                        = (fmi3ResetTYPE*)  LoadFMUFcnStatus(fmustruct, "fmi3Reset", 0);
        /* Startup/cleanup */
        fmustruct->freeInstance                 = (fmi3FreeInstanceTYPE*)  LoadFMUFcnVoid(fmustruct, "fmi3FreeInstance", 0);
        /* data marshalling */
        fmustruct->getFloat32                   = (fmi3GetFloat32TYPE*)  LoadFMUFcnStatus(fmustruct, "fmi3GetFloat32", 0);
        fmustruct->getFloat64                   = (fmi3GetFloat64TYPE*)  LoadFMUFcnStatus(fmustruct, "fmi3GetFloat64", 0);
        fmustruct->getInt8                      = (fmi3GetInt8TYPE*)  LoadFMUFcnStatus(fmustruct, "fmi3GetInt8", 0);
        fmustruct->getUInt8                     = (fmi3GetUInt8TYPE*)  LoadFMUFcnStatus(fmustruct, "fmi3GetUInt8", 0);
        fmustruct->getInt16                     = (fmi3GetInt16TYPE*)  LoadFMUFcnStatus(fmustruct, "fmi3GetInt16", 0);
        fmustruct->getUInt16                    = (fmi3GetUInt16TYPE*)  LoadFMUFcnStatus(fmustruct, "fmi3GetUInt16", 0);
        fmustruct->getInt32                     = (fmi3GetInt32TYPE*)  LoadFMUFcnStatus(fmustruct, "fmi3GetInt32", 0);
        fmustruct->getUInt32                    = (fmi3GetUInt32TYPE*)  LoadFMUFcnStatus(fmustruct, "fmi3GetUInt32", 0);
        fmustruct->getInt64                     = (fmi3GetInt64TYPE*)  LoadFMUFcnStatus(fmustruct, "fmi3GetInt64", 0);
        fmustruct->getUInt64                    = (fmi3GetUInt64TYPE*)  LoadFMUFcnStatus(fmustruct, "fmi3GetUInt64", 0);
        fmustruct->getBoolean                   = (fmi3GetBooleanTYPE*)  LoadFMUFcnStatus(fmustruct, "fmi3GetBoolean", 0);
        fmustruct->getString                    = (fmi3GetStringTYPE*)  LoadFMUFcnStatus(fmustruct, "fmi3GetString", 0);
        fmustruct->getBinary                    = (fmi3GetBinaryTYPE*)  LoadFMUFcnStatus(fmustruct, "fmi3GetBinary", 0);
        fmustruct->getClock                     = (fmi3GetClockTYPE*)  LoadFMUFcnStatus(fmustruct, "fmi3GetClock", 0);
        fmustruct->setFloat32                   = (fmi3SetFloat32TYPE*)  LoadFMUFcnStatus(fmustruct, "fmi3SetFloat32", 0);
        fmustruct->setFloat64                   = (fmi3SetFloat64TYPE*)  LoadFMUFcnStatus(fmustruct, "fmi3SetFloat64", 0);
        fmustruct->setInt8                      = (fmi3SetInt8TYPE*)  LoadFMUFcnStatus(fmustruct, "fmi3SetInt8", 0);
        fmustruct->setUInt8                     = (fmi3SetUInt8TYPE*)  LoadFMUFcnStatus(fmustruct, "fmi3SetUInt8", 0);
        fmustruct->setInt16                     = (fmi3SetInt16TYPE*)  LoadFMUFcnStatus(fmustruct, "fmi3SetInt16", 0);
        fmustruct->setUInt16                    = (fmi3SetUInt16TYPE*)  LoadFMUFcnStatus(fmustruct, "fmi3SetUInt16", 0);
        fmustruct->setInt32                     = (fmi3SetInt32TYPE*)  LoadFMUFcnStatus(fmustruct, "fmi3SetInt32", 0);
        fmustruct->setUInt32                    = (fmi3SetUInt32TYPE*)  LoadFMUFcnStatus(fmustruct, "fmi3SetUInt32", 0);
        fmustruct->setInt64                     = (fmi3SetInt64TYPE*)  LoadFMUFcnStatus(fmustruct, "fmi3SetInt64", 0);
        fmustruct->setUInt64                    = (fmi3SetUInt64TYPE*)  LoadFMUFcnStatus(fmustruct, "fmi3SetUInt64", 0);
        fmustruct->setBoolean                   = (fmi3SetBooleanTYPE*)  LoadFMUFcnStatus(fmustruct, "fmi3SetBoolean", 0);
        fmustruct->setString                    = (fmi3SetStringTYPE*)  LoadFMUFcnStatus(fmustruct, "fmi3SetString", 0);
        fmustruct->setBinary                    = (fmi3SetBinaryTYPE*)  LoadFMUFcnStatus(fmustruct, "fmi3SetBinary", 0);
        fmustruct->setClock                     = (fmi3SetClockTYPE*)  LoadFMUFcnStatus(fmustruct, "fmi3SetClock", 0);
        /* Event */
        fmustruct->setIntervalDecimal           = (fmi3SetIntervalDecimalTYPE*)  LoadFMUFcnStatus(fmustruct, "fmi3SetIntervalDecimal", 0);
        fmustruct->setIntervalFraction          = (fmi3SetIntervalFractionTYPE*)  LoadFMUFcnStatus(fmustruct, "fmi3SetIntervalFraction", 0);
        fmustruct->getIntervalDecimal           = (fmi3GetIntervalDecimalTYPE*)  LoadFMUFcnStatus(fmustruct, "fmi3GetIntervalDecimal", 0);
        fmustruct->getIntervalFraction          = (fmi3GetIntervalFractionTYPE*)  LoadFMUFcnStatus(fmustruct, "fmi3GetIntervalFraction", 0);
        fmustruct->getShiftDecimal              = (fmi3GetShiftDecimalTYPE*)  LoadFMUFcnStatus(fmustruct, "fmi3GetShiftDecimal", 0);
        fmustruct->getShiftFraction             = (fmi3GetShiftFractionTYPE*)  LoadFMUFcnStatus(fmustruct, "fmi3GetShiftFraction", 0);
        fmustruct->setShiftDecimal              = (fmi3SetShiftDecimalTYPE*)  LoadFMUFcnStatus(fmustruct, "fmi3SetShiftDecimal", 0);
        fmustruct->setShiftFraction             = (fmi3SetShiftFractionTYPE*)  LoadFMUFcnStatus(fmustruct, "fmi3SetShiftFraction", 0);
        /* Save/restore */
        fmustruct->getFMUState                      = (fmi3GetFMUStateTYPE*)  LoadFMUFcnStatus(fmustruct, "fmi3GetFMUState", !loadFMUStateFcn);
        fmustruct->setFMUState                      = (fmi3SetFMUStateTYPE*)  LoadFMUFcnStatus(fmustruct, "fmi3SetFMUState", !loadFMUStateFcn);
        fmustruct->freeFMUState                     = (fmi3FreeFMUStateTYPE*)  LoadFMUFcnStatus(fmustruct, "fmi3FreeFMUState", !loadFMUStateFcn);
        fmustruct->serializedFMUStateSize           = (fmi3SerializedFMUStateSizeTYPE*)  LoadFMUFcnStatus(fmustruct, "fmi3SerializedFMUStateSize", !loadSerializationFcn);
        fmustruct->serializeFMUState                = (fmi3SerializeFMUStateTYPE*)  LoadFMUFcnStatus(fmustruct, "fmi3SerializeFMUState", !loadSerializationFcn);
        fmustruct->deserializeFMUState              = (fmi3DeserializeFMUStateTYPE*)  LoadFMUFcnStatus(fmustruct, "fmi3DeserializeFMUState", !loadSerializationFcn);
        /* Model structure */
        fmustruct->getNumberOfVariableDependencies  = (fmi3GetNumberOfVariableDependenciesTYPE*)  LoadFMUFcnStatus(fmustruct, "fmi3GetNumberOfVariableDependencies", 0);
        fmustruct->getVariableDependencies          = (fmi3GetVariableDependenciesTYPE*)  LoadFMUFcnStatus(fmustruct, "fmi3GetVariableDependencies", 0);
        /* Jacobian */
        fmustruct->getDirectionalDerivative         = (fmi3GetDirectionalDerivativeTYPE*)  LoadFMUFcnStatus(fmustruct, "fmi3GetDirectionalDerivative", !loadDirectionDerivativeFcn);
        fmustruct->getAdjointDerivative             = (fmi3GetAdjointDerivativeTYPE*)  LoadFMUFcnStatus(fmustruct, "fmi3GetAdjointDerivative", !loadDirectionDerivativeFcn);
        /* ME/CS semantics */
        fmustruct->enterEventMode                   = (fmi3EnterEventModeTYPE*)  LoadFMUFcnStatus(fmustruct, "fmi3EnterEventMode", 0);
        fmustruct->updateDiscreteStates             = (fmi3UpdateDiscreteStatesTYPE*)  LoadFMUFcnStatus(fmustruct, "fmi3UpdateDiscreteStates", 0);
        /* Intermediate update */
        fmustruct->evaluateDiscreteStates           = (fmi3EvaluateDiscreteStatesTYPE*)  LoadFMUFcnStatus(fmustruct, "fmi3EvaluateDiscreteStates", 0);
}

/*Load FMU 3 CS Functions*/
static void LoadFMU3CSFunctions(struct FMU3_CSME_RTWCG* fmustruct) {
    /* Startup/cleanup */
    fmustruct->instantiateCoSimulation  = (fmi3InstantiateCoSimulationTYPE*)  LoadFMUFcnInstance(fmustruct, "fmi3InstantiateCoSimulation", 0);
    /* CS semantics */
    fmustruct->enterStepMode            = (fmi3EnterStepModeTYPE*)  LoadFMUFcnStatus(fmustruct, "fmi3EnterStepMode", 0);
    fmustruct->doStep                   = (fmi3DoStepTYPE*)  LoadFMUFcnStatus(fmustruct, "fmi3DoStep", 0);
    /* Compensation */
    fmustruct->getOutputDerivatives     = (fmi3GetOutputDerivativesTYPE*)  LoadFMUFcnStatus(fmustruct, "fmi3GetOutputDerivatives", 0);
}

/*Load FMU 3 ME Functions*/
static void LoadFMU3MEFunctions(struct FMU3_CSME_RTWCG* fmustruct) {
    /* Startup/cleanup */
    fmustruct->instantiateModelExchange             = (fmi3InstantiateModelExchangeTYPE*)  LoadFMUFcnInstance(fmustruct, "fmi3InstantiateModelExchange", 0);
    /* ME semantics */
    fmustruct->getNumberOfContinuousStates          = (fmi3GetNumberOfContinuousStatesTYPE*)  LoadFMUFcnStatus(fmustruct, "fmi3GetNumberOfContinuousStates", 0);
    fmustruct->getNumberOfEventIndicators           = (fmi3GetNumberOfEventIndicatorsTYPE*)  LoadFMUFcnStatus(fmustruct, "fmi3GetNumberOfEventIndicators", 0);
    fmustruct->enterContinuousTimeMode              = (fmi3EnterContinuousTimeModeTYPE*)  LoadFMUFcnStatus(fmustruct, "fmi3EnterContinuousTimeMode", 0);
    fmustruct->getContinuousStateDerivatives        = (fmi3GetContinuousStateDerivativesTYPE*)  LoadFMUFcnStatus(fmustruct, "fmi3GetContinuousStateDerivatives", 0);
    fmustruct->getContinuousStates                  = (fmi3GetContinuousStatesTYPE*)  LoadFMUFcnStatus(fmustruct, "fmi3GetContinuousStates", 0);
    fmustruct->getNominalsOfContinuousStates        = (fmi3GetNominalsOfContinuousStatesTYPE*)  LoadFMUFcnStatus(fmustruct, "fmi3GetNominalsOfContinuousStates", 0);
    fmustruct->getEventIndicators                   = (fmi3GetEventIndicatorsTYPE*)  LoadFMUFcnStatus(fmustruct, "fmi3GetEventIndicators", 0);
    fmustruct->setTime                              = (fmi3SetTimeTYPE*)  LoadFMUFcnStatus(fmustruct, "fmi3SetTime", 0);
    fmustruct->setContinuousStates                  = (fmi3SetContinuousStatesTYPE*)  LoadFMUFcnStatus(fmustruct, "fmi3SetContinuousStates", 0);
    fmustruct->completedIntegratorStep              = (fmi3CompletedIntegratorStepTYPE*)  LoadFMUFcnStatus(fmustruct, "fmi3CompletedIntegratorStep" , 0);
}

static fmi3Boolean CheckStatus(struct FMU3_CSME_RTWCG* fmustruct,
                               fmi3Status status,
                               fmi3String fcnName) {

#if FMU_CG_TARGET < FMUCG_STANDALONE_TARGETS
    SimStruct* ss = fmustruct->ssPtr;

    if(status == fmi3Error || status == fmi3Fatal){
        if (fmustruct->isLoggingOn) {
            void * diagnostic = CreateDiagnosticAsVoidPtr("FMUBlock:FMU:FMUSimFunctionErrorDebugToDisplayOn", 2,
                                                          CODEGEN_SUPPORT_ARG_STRING_TYPE, fcnName,
                                                          CODEGEN_SUPPORT_ARG_STRING_TYPE, fmustruct->fmuname);
            rt_ssSet_slErrMsg(ss, diagnostic);
        }
        fmustruct->FMUErrorStatus = status;
        ssSetStopRequested(ss, 1);
    }
#else
    fmi3String message = NULL;
    if(status == fmi3Error || status == fmi3Fatal){
        fmustruct->FMUErrorStatus = status;
        message = formatString("Error in supplied FMU: An error occurred in function '%s' for block '%s' during simulation.", fcnName, fmustruct->fmuname);
        fmustruct->logMessage(fmustruct->instanceEnvironment, fmustruct->FMUErrorStatus, fcnName, message);
    }
#endif

    if (status == fmi3OK)
        return fmi3True;
    else
        return fmi3False;
}

static void* FMU3_fmuInitialize_common(const char* lib,
                                fmi3String instanceName,
                                fmi3InstanceEnvironment instanceEnvironment,
                                void* ssPtr,
                                int loadFMUStateFcn,
                                int loadSerializationFcn,
                                int loadDirectionDerivativeFcn,
                                fmi3Boolean loggingOn,
                                fmi3LogMessageCallback logMessage) {

    struct FMU3_CSME_RTWCG * fmustruct;

    fmi3String message = NULL;

    fmustruct = (struct FMU3_CSME_RTWCG *)calloc(1, sizeof(struct FMU3_CSME_RTWCG));
    fmustruct->logMessage = (logMessage != NULL) ? logMessage : FMU3_Logger;
    fmustruct->fmuname = (instanceName != NULL) ? (char *)instanceName : "";
    fmustruct->instanceEnvironment = (logMessage != NULL) ? instanceEnvironment : fmustruct;
    fmustruct->dllfile = (char *)lib;
    fmustruct->FMUErrorStatus = fmi3OK;
    fmustruct->modelInitialized = false;
    fmustruct->isLoggingOn = loggingOn;

#if FMU_CG_TARGET < FMUCG_STANDALONE_TARGETS
    fmustruct->ssPtr =  (SimStruct*)ssPtr;
    (void) message;
#else
    UNUSED_PARAMETER(ssPtr);
#endif

    if (strlen(instanceName)+ FCN_NAME_MAX_LEN + 1 >= FULL_FCN_NAME_MAX_LEN) {
        /*FMU name is longer than 200+, rarely happens*/
        fmustruct->FMUErrorStatus = fmi3Fatal;
#if FMU_CG_TARGET < FMUCG_STANDALONE_TARGETS
        if (fmustruct->isLoggingOn) {
            void *diagnostic = CreateDiagnosticAsVoidPtr("SL_SERVICES:utils:PRINTFWRAPPER", 1,
                                                         CODEGEN_SUPPORT_ARG_STRING_TYPE, "FMU Name is too long.");
            rt_ssReportDiagnosticAsWarning(fmustruct->ssPtr, diagnostic);
        }
        ssSetStopRequested(fmustruct->ssPtr, 1);
#else
        message = formatString("FMU Name '%s' is too long.", instanceName);
        fmustruct->logMessage(fmustruct->instanceEnvironment, fmustruct->FMUErrorStatus, "FMU3_fmuInitialize", message);
#endif
        return NULL;
    }

    fmustruct->Handle = LOAD_LIBRARY(fmustruct->dllfile);
    if (NULL == fmustruct->Handle) {
        fmustruct->FMUErrorStatus = fmi3Fatal;
#if FMU_CG_TARGET < FMUCG_STANDALONE_TARGETS
        if (fmustruct->isLoggingOn) {
            void *diagnostic = CreateDiagnosticAsVoidPtr("FMUBlock:FMU:FMULoadLibraryError", 2,
                                                         CODEGEN_SUPPORT_ARG_STRING_TYPE, fmustruct->dllfile,
                                                         CODEGEN_SUPPORT_ARG_STRING_TYPE, fmustruct->fmuname);
            /*loading lib failure will halt simulation*/
            rt_ssReportDiagnosticAsWarning(fmustruct->ssPtr, diagnostic);
        }
        ssSetStopRequested(fmustruct->ssPtr, 1);
#else
        const char* errorCodeMessage = getLoadLibraryErrorMessage();
        message = formatString("Unable to load dynamic library: '%s'. %s", fmustruct->dllfile, errorCodeMessage);
        fmustruct->logMessage(fmustruct->instanceEnvironment, fmustruct->FMUErrorStatus, "FMU3_fmuInitialize", message);
#endif
        return NULL;
    }

    /* load fmi functions */
    LoadFMU3CommonFunctions(fmustruct, loadFMUStateFcn, loadSerializationFcn, loadDirectionDerivativeFcn);
    return (void *) fmustruct;
}

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
                           fmi3IntermediateUpdateCallback intermediateUpdate
                           ) {

    struct FMU3_CSME_RTWCG* fmustruct =  FMU3_fmuInitialize_common(lib,
                              instanceName,
                              instanceEnvironment,
                              ssPtr,
                              loadFMUStateFcn,
                              loadSerializationFcn,
                              loadDirectionDerivativeFcn,
                              loggingOn,
                              logMessage);

    LoadFMU3CSFunctions(fmustruct);

    /* instantiate fmu */
    fmustruct->mFMIComp = fmustruct->instantiateCoSimulation(instanceName,
                                                 instantiationToken,
                                                 resourcePath,
                                                 visible,
                                                 loggingOn,
                                                 eventModeUsed,
                                                 earlyReturnAllowed,
                                                 requiredIntermediateVariables,
                                                 nRequiredIntermediateVariables,
                                                 fmustruct->instanceEnvironment,
                                                 fmustruct->logMessage,
                                                 intermediateUpdate // null_ptr
                                                 );
    if (NULL == fmustruct->mFMIComp ){
        CheckStatus(fmustruct, fmi3Error, "fmi3Instantiate");
        CLOSE_LIBRARY(fmustruct->Handle);
        free(fmustruct);
        return NULL;
    }
    return (void *) fmustruct;
}

void* FMU3_fmuInitializeME(const char* lib,
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
                           fmi3LogMessageCallback logMessage) {

    struct FMU3_CSME_RTWCG* fmustruct =  FMU3_fmuInitialize_common(lib,
                              instanceName,
                              instanceEnvironment,
                              ssPtr,
                              loadFMUStateFcn,
                              loadSerializationFcn,
                              loadDirectionDerivativeFcn,
                              loggingOn,
                              logMessage);

    LoadFMU3MEFunctions(fmustruct);

    /* instantiate fmu */
    fmustruct->mFMIComp = fmustruct->instantiateModelExchange(instanceName,
                                                 instantiationToken,
                                                 resourcePath,
                                                 visible,
                                                 loggingOn,
                                                 fmustruct->instanceEnvironment,
                                                 fmustruct->logMessage
                                                 );
    if (NULL == fmustruct->mFMIComp ){
        CheckStatus(fmustruct, fmi3Error, "fmi3Instantiate");
        CLOSE_LIBRARY(fmustruct->Handle);
        free(fmustruct);
        return NULL;
    }
    return (void *) fmustruct;
}

fmi3Boolean FMU3_enterInitializationMode(void** fmuv) {
    struct FMU3_CSME_RTWCG * fmustruct = (struct FMU3_CSME_RTWCG *)(*fmuv);
    // FIXME fmu3codegen_tag
    fmi3Boolean toleranceDefined = 0;
    fmi3Float64 tolerance = 0;
    fmi3Float64 startTime = 0;
    fmi3Boolean stopTimeDefined = 0;
    fmi3Float64 stopTime = 0;
    fmi3Status fmi3Flag = fmustruct->enterInitializationMode(fmustruct->mFMIComp, toleranceDefined, tolerance, startTime, stopTimeDefined, stopTime);
    return CheckStatus(fmustruct, fmi3Flag, "fmi3EnterInitializationMode");
}
fmi3Boolean FMU3_exitInitializationMode(void** fmuv) {
    struct FMU3_CSME_RTWCG * fmustruct = (struct FMU3_CSME_RTWCG *)(*fmuv);
    fmi3Status fmi3Flag = fmustruct->exitInitializationMode(fmustruct->mFMIComp);
    fmustruct->modelInitialized = true;
    return CheckStatus(fmustruct, fmi3Flag, "fmi3ExitInitializationMode");
}

fmi3Boolean FMU3_terminate(void **fmuv) {
    struct FMU3_CSME_RTWCG * fmustruct = (struct FMU3_CSME_RTWCG *)(*fmuv);
    fmi3Boolean returnStatus = fmi3True;
    if (fmustruct == NULL) {
        return returnStatus;
    }
    if(fmustruct->FMUErrorStatus != fmi3Fatal){
       if(fmustruct->FMUErrorStatus != fmi3Error &&
          fmustruct->modelInitialized == true) {
            fmi3Status fmi3Flag = fmustruct->terminate(fmustruct->mFMIComp);
            returnStatus = CheckStatus(fmustruct, fmi3Flag, "fmi3TerminateSlave");
        }
        fmustruct->freeInstance(fmustruct->mFMIComp);
    }
    if (fmustruct->Handle != NULL) {
        CLOSE_LIBRARY(fmustruct->Handle);
    }
    free(fmustruct->paramIdxToOffset);
    free(fmustruct->enumValueList);
    free(fmustruct);
    return returnStatus;
}

void FMU3_setFMUState(void** fmuv, void* fmustate){
    struct FMU3_CSME_RTWCG * fmustruct = (struct FMU3_CSME_RTWCG *)(*fmuv);
    fmustruct->setFMUState(fmustruct->mFMIComp, fmustate);
}
void FMU3_getFMUState(void** fmuv, void** fmustate){
    struct FMU3_CSME_RTWCG * fmustruct = (struct FMU3_CSME_RTWCG *)(*fmuv);
    fmustruct->getFMUState(fmustruct->mFMIComp, fmustate);
}

void FMU3_freeFMUState(void** fmuv, void** fmustate){
    struct FMU3_CSME_RTWCG * fmustruct = (struct FMU3_CSME_RTWCG *)(*fmuv);
    fmustruct->freeFMUState(fmustruct->mFMIComp, fmustate);
}

void FMU3_serializedFMUstateSize(void** fmuv, void* fmustate, size_t* size){
    struct FMU3_CSME_RTWCG * fmustruct = (struct FMU3_CSME_RTWCG *)(*fmuv);
    fmustruct->serializedFMUStateSize(fmustruct->mFMIComp, fmustate, size);
}

void FMU3_serializeFMUstate(void** fmuv, void* fmustate, char* byteArray){
    struct FMU3_CSME_RTWCG * fmustruct = (struct FMU3_CSME_RTWCG *)(*fmuv);
    size_t size = 0;
    FMU3_serializedFMUstateSize(fmuv, fmustate, &size);
    fmustruct->serializeFMUState(fmustruct->mFMIComp, fmustate, (fmi3Byte*)byteArray, size);
}

void FMU3_deserializeFMUstate(void** fmuv, const char* byteArray, size_t size, void** fmustate){
    struct FMU3_CSME_RTWCG * fmustruct = (struct FMU3_CSME_RTWCG *)(*fmuv);
    fmustruct->deserializeFMUState(fmustruct->mFMIComp, (fmi3Byte*)byteArray, size, fmustate);
}

fmi3Boolean FMU3_doStep(void **fmuv,
                       fmi3Float64 currentCommunicationPoint,
                       fmi3Float64 communicationStepSize,
                       fmi3Boolean noSetFMUStatePriorToCurrentPoint,
                       void* eventHandlingNeeded) {
    fmi3String message = NULL;
#if FMU_CG_TARGET < FMUCG_STANDALONE_TARGETS
    (void) message;
#endif
    struct FMU3_CSME_RTWCG * fmustruct = (struct FMU3_CSME_RTWCG *)(*fmuv);
    // FIXME fmu3codegen_tag support event mode and earlyReturn
    fmi3Boolean terminateSimulation = 0;
    fmi3Boolean earlyReturn = 0;
    fmi3Float64 lastSuccessfulTime = 0;

    fmi3Status fmi3Flag = fmustruct->doStep(fmustruct->mFMIComp, currentCommunicationPoint,communicationStepSize, noSetFMUStatePriorToCurrentPoint, (fmi3Boolean*)eventHandlingNeeded, &terminateSimulation, &earlyReturn, &lastSuccessfulTime);
    if(fmi3Flag == fmi3Discard){
         if(terminateSimulation == fmi3True) {
             char time[20];
             memset(time, 0, 20);
             snprintf(time, 20, "%f", currentCommunicationPoint);
#if FMU_CG_TARGET < FMUCG_STANDALONE_TARGETS
             if (fmustruct->isLoggingOn) {
                 void *diagnostic = CreateDiagnosticAsVoidPtr("FMUBlock:FMU3:FMU3SimDoStepTerminated", 2,
                                                              CODEGEN_SUPPORT_ARG_STRING_TYPE, fmustruct->fmuname,
                                                              CODEGEN_SUPPORT_ARG_STRING_TYPE, time);
                 rt_ssReportDiagnosticAsInfo(fmustruct->ssPtr, diagnostic);
             }
             ssSetStopRequested(fmustruct->ssPtr, 1);
#else
            message = formatString("'fmi3Terminated' flag was set to true for FMU '%s' after 'fmi3DoStep' function returns 'fmi3Discard' at time %f.", fmustruct->fmuname, time);
            fmustruct->logMessage(fmustruct->instanceEnvironment, fmustruct->FMUErrorStatus, "FMU3_doStep", message);
#endif
         }
    }
    return CheckStatus(fmustruct, fmi3Flag, "fmi3DoStep");
}

fmi3Boolean FMU3_setFloat64Ptr(void **fmuv,
                           const fmi3ValueReference dvr,
                           size_t nvr,
                           const fmi3Float64 value[],
                           size_t nVal) {
    struct FMU3_CSME_RTWCG * fmustruct = (struct FMU3_CSME_RTWCG *)(*fmuv);
    fmi3ValueReference vr =dvr;
    fmi3Status fmi3Flag = fmustruct->setFloat64(fmustruct->mFMIComp, &vr, nvr, value, nVal);
    return CheckStatus(fmustruct, fmi3Flag, "fmi3setFloat64");
}

fmi3Boolean FMU3_setFloat32Ptr(void **fmuv,
                           const fmi3ValueReference dvr,
                           size_t nvr,
                           const fmi3Float32 value[],
                           size_t nVal) {
    struct FMU3_CSME_RTWCG * fmustruct = (struct FMU3_CSME_RTWCG *)(*fmuv);
    fmi3ValueReference vr =dvr;
    fmi3Status fmi3Flag = fmustruct->setFloat32(fmustruct->mFMIComp, &vr, nvr, value ,nVal);
    return CheckStatus(fmustruct, fmi3Flag, "fmi3setFloat32");
}

fmi3Boolean FMU3_setInt8Ptr(void **fmuv,
                           const fmi3ValueReference dvr,
                           size_t nvr,
                         const fmi3Int8 value[],
                         size_t nVal) {
    struct FMU3_CSME_RTWCG * fmustruct = (struct FMU3_CSME_RTWCG *)(*fmuv);
    fmi3ValueReference vr =dvr;
    fmi3Status fmi3Flag = fmustruct->setInt8(fmustruct->mFMIComp, &vr, nvr, value ,nVal);
    return CheckStatus(fmustruct, fmi3Flag, "fmi3setInt8");
}

fmi3Boolean FMU3_setUInt8Ptr(void **fmuv,
                           const fmi3ValueReference dvr,
                           size_t nvr,
                          const fmi3UInt8 value[],
                          size_t nVal) {
    struct FMU3_CSME_RTWCG * fmustruct = (struct FMU3_CSME_RTWCG *)(*fmuv);
    fmi3ValueReference vr =dvr;
    fmi3Status fmi3Flag = fmustruct->setUInt8(fmustruct->mFMIComp, &vr, nvr, value ,nVal);
    return CheckStatus(fmustruct, fmi3Flag, "fmi3setUInt8");
}

fmi3Boolean FMU3_setInt16Ptr(void **fmuv,
                           const fmi3ValueReference dvr,
                           size_t nvr,
                          const fmi3Int16 value[],
                          size_t nVal) {
    struct FMU3_CSME_RTWCG * fmustruct = (struct FMU3_CSME_RTWCG *)(*fmuv);
    fmi3ValueReference vr =dvr;
    fmi3Status fmi3Flag = fmustruct->setInt16(fmustruct->mFMIComp, &vr, nvr, value ,nVal);
    return CheckStatus(fmustruct, fmi3Flag, "fmi3setInt16");
}

fmi3Boolean FMU3_setUInt16Ptr(void **fmuv,
                           const fmi3ValueReference dvr,
                           size_t nvr,
                          const fmi3UInt16 value[],
                          size_t nVal) {
    struct FMU3_CSME_RTWCG * fmustruct = (struct FMU3_CSME_RTWCG *)(*fmuv);
    fmi3ValueReference vr =dvr;
    fmi3Status fmi3Flag = fmustruct->setUInt16(fmustruct->mFMIComp, &vr, nvr, value ,nVal);
    return CheckStatus(fmustruct, fmi3Flag, "fmi3setUInt16");
}

fmi3Boolean FMU3_setInt32Ptr(void **fmuv,
                           const fmi3ValueReference dvr,
                           size_t nvr,
                          const fmi3Int32 value[],
                          size_t nVal) {
    struct FMU3_CSME_RTWCG * fmustruct = (struct FMU3_CSME_RTWCG *)(*fmuv);
    fmi3ValueReference vr =dvr;
    fmi3Status fmi3Flag = fmustruct->setInt32(fmustruct->mFMIComp, &vr, nvr, value ,nVal);
    return CheckStatus(fmustruct, fmi3Flag, "fmi3setInt32");
}

fmi3Boolean FMU3_setUInt32Ptr(void **fmuv,
                           const fmi3ValueReference dvr,
                           size_t nvr,
                           const fmi3UInt32 value[],
                           size_t nVal) {
    struct FMU3_CSME_RTWCG * fmustruct = (struct FMU3_CSME_RTWCG *)(*fmuv);
    fmi3ValueReference vr =dvr;
    fmi3Status fmi3Flag = fmustruct->setUInt32(fmustruct->mFMIComp, &vr, nvr, value ,nVal);
    return CheckStatus(fmustruct, fmi3Flag, "fmi3setUInt32");
}

fmi3Boolean FMU3_setInt64EnumPtr(void **fmuv,
                           const fmi3ValueReference dvr,
                           size_t nvr,
                           const void* value,
                           size_t nVal) {
    struct FMU3_CSME_RTWCG * fmustruct = (struct FMU3_CSME_RTWCG *)(*fmuv);
    fmi3ValueReference vr =dvr;
    fmi3Status fmi3Flag = fmustruct->setInt64(fmustruct->mFMIComp, &vr, nvr, (const fmi3Int64*)value ,nVal);
    return CheckStatus(fmustruct, fmi3Flag, "fmi3setInt64");
}

fmi3Boolean FMU3_setUInt64Ptr(void **fmuv,
                           const fmi3ValueReference dvr,
                           size_t nvr,
                           const void* value,
                           size_t nVal) {
    struct FMU3_CSME_RTWCG * fmustruct = (struct FMU3_CSME_RTWCG *)(*fmuv);
    fmi3ValueReference vr =dvr;
    fmi3Status fmi3Flag = fmustruct->setUInt64(fmustruct->mFMIComp, &vr, nvr, (const fmi3UInt64*)value ,nVal);
    return CheckStatus(fmustruct, fmi3Flag, "fmi3setUInt64");
}

fmi3Boolean FMU3_setBooleanPtr(void **fmuv,
                            const fmi3ValueReference dvr,
                            size_t nvr,
                            const fmi3UInt8 value[],
                            size_t nVal) {
    struct FMU3_CSME_RTWCG * fmustruct = (struct FMU3_CSME_RTWCG *)(*fmuv);
    fmi3ValueReference vr =dvr;
    fmi3Status fmi3Flag = fmustruct->setBoolean(fmustruct->mFMIComp, &vr, nvr, (const fmi3Boolean*)value ,nVal);
    return CheckStatus(fmustruct, fmi3Flag, "fmi3setBoolean");
}

fmi3Boolean FMU3_setStringPtr(void **fmuv,
                           const fmi3ValueReference dvr,
                           size_t nvr,
                           const fmi3String value[]) {
    struct FMU3_CSME_RTWCG * fmustruct = (struct FMU3_CSME_RTWCG *)(*fmuv);
    fmi3ValueReference vr =dvr;
    fmi3Status fmi3Flag = fmustruct->setString(fmustruct->mFMIComp, &vr, nvr, value ,nvr);
    return CheckStatus(fmustruct, fmi3Flag, "fmi3setString");
}


fmi3Boolean FMU3_setFloat64(void **fmuv,
                        const fmi3ValueReference vr[],
                        size_t nvr,
                        const fmi3Float64 value[], size_t nVal) {
    struct FMU3_CSME_RTWCG * fmustruct = (struct FMU3_CSME_RTWCG *)(*fmuv);
    fmi3Status fmi3Flag = fmustruct->setFloat64(fmustruct->mFMIComp, vr, nvr, value, nVal);
    return CheckStatus(fmustruct, fmi3Flag, "fmi3Float64");
}

fmi3Boolean FMU3_setFloat32(void **fmuv,
                        const fmi3ValueReference vr[],
                        size_t nvr,
                        const fmi3Float32 value[],
                        size_t nVal) {
    struct FMU3_CSME_RTWCG * fmustruct = (struct FMU3_CSME_RTWCG *)(*fmuv);
    fmi3Status fmi3Flag = fmustruct->setFloat32(fmustruct->mFMIComp, vr, nvr, value, nVal);
    return CheckStatus(fmustruct, fmi3Flag, "fmi3Float32");
}

fmi3Boolean FMU3_setInt8(void **fmuv,
                        const fmi3ValueReference vr[],
                        size_t nvr,
                        const fmi3Int8 value[],
                        size_t nVal) {
    struct FMU3_CSME_RTWCG * fmustruct = (struct FMU3_CSME_RTWCG *)(*fmuv);
    fmi3Status fmi3Flag = fmustruct->setInt8(fmustruct->mFMIComp, vr, nvr, value, nVal);
    return CheckStatus(fmustruct, fmi3Flag, "fmi3Int8");
}

fmi3Boolean FMU3_setUInt8(void **fmuv,
                        const fmi3ValueReference vr[],
                        size_t nvr,
                        const fmi3UInt8 value[],
                        size_t nVal) {
    struct FMU3_CSME_RTWCG * fmustruct = (struct FMU3_CSME_RTWCG *)(*fmuv);
    fmi3Status fmi3Flag = fmustruct->setUInt8(fmustruct->mFMIComp, vr, nvr, value, nVal);
    return CheckStatus(fmustruct, fmi3Flag, "fmi3UInt8");
}

fmi3Boolean FMU3_setInt16(void **fmuv,
                        const fmi3ValueReference vr[],
                        size_t nvr,
                        const fmi3Int16 value[],
                        size_t nVal) {
    struct FMU3_CSME_RTWCG * fmustruct = (struct FMU3_CSME_RTWCG *)(*fmuv);
    fmi3Status fmi3Flag = fmustruct->setInt16(fmustruct->mFMIComp, vr, nvr, value, nVal);
    return CheckStatus(fmustruct, fmi3Flag, "fmi3Int16");
}

fmi3Boolean FMU3_setUInt16(void **fmuv,
                        const fmi3ValueReference vr[],
                        size_t nvr,
                        const fmi3UInt16 value[],
                        size_t nVal) {
    struct FMU3_CSME_RTWCG * fmustruct = (struct FMU3_CSME_RTWCG *)(*fmuv);
    fmi3Status fmi3Flag = fmustruct->setUInt16(fmustruct->mFMIComp, vr, nvr, value, nVal);
    return CheckStatus(fmustruct, fmi3Flag, "fmi3UInt16");
}

fmi3Boolean FMU3_setInt32(void **fmuv,
                        const fmi3ValueReference vr[],
                        size_t nvr,
                        const fmi3Int32 value[],
                        size_t nVal) {
    struct FMU3_CSME_RTWCG * fmustruct = (struct FMU3_CSME_RTWCG *)(*fmuv);
    fmi3Status fmi3Flag = fmustruct->setInt32(fmustruct->mFMIComp, vr, nvr, value, nVal);
    return CheckStatus(fmustruct, fmi3Flag, "fmi3Int32");
}

fmi3Boolean FMU3_setUInt32(void **fmuv,
                        const fmi3ValueReference vr[],
                        size_t nvr,
                        const fmi3UInt32 value[],
                        size_t nVal) {
    struct FMU3_CSME_RTWCG * fmustruct = (struct FMU3_CSME_RTWCG *)(*fmuv);
    fmi3Status fmi3Flag = fmustruct->setUInt32(fmustruct->mFMIComp, vr, nvr, value, nVal);
    return CheckStatus(fmustruct, fmi3Flag, "fmi3UInt32");
}

fmi3Boolean FMU3_setInt64Enum(void **fmuv,
                        const fmi3ValueReference vr[],
                        size_t nvr,
                        const void* value,
                        size_t nVal) {
    struct FMU3_CSME_RTWCG * fmustruct = (struct FMU3_CSME_RTWCG *)(*fmuv);
    fmi3Status fmi3Flag = fmustruct->setInt64(fmustruct->mFMIComp, vr, nvr, (const fmi3Int64*)value, nVal);
    return CheckStatus(fmustruct, fmi3Flag, "fmi3Int64");
}

fmi3Boolean FMU3_setUInt64(void **fmuv,
                        const fmi3ValueReference vr[],
                        size_t nvr,
                        const void* value,
                        size_t nVal) {
    struct FMU3_CSME_RTWCG * fmustruct = (struct FMU3_CSME_RTWCG *)(*fmuv);
    fmi3Status fmi3Flag = fmustruct->setUInt64(fmustruct->mFMIComp, vr, nvr, (const fmi3UInt64*)value, nVal);
    return CheckStatus(fmustruct, fmi3Flag, "fmi3UInt64");
}

fmi3Boolean FMU3_setBoolean(void **fmuv,
                           const fmi3ValueReference vr[],
                           size_t nvr,
                           const fmi3UInt8 value[],
                           size_t nVal) {
    struct FMU3_CSME_RTWCG * fmustruct = (struct FMU3_CSME_RTWCG *)(*fmuv);
    fmi3Status fmi3Flag = fmustruct->setBoolean(fmustruct->mFMIComp, vr, nvr, (const fmi3Boolean*)value ,nVal);
    return CheckStatus(fmustruct, fmi3Flag, "fmi3SetBoolean");
}

fmi3Boolean FMU3_setString(void **fmuv,
                          const fmi3ValueReference vr[],
                          size_t nvr,
                          void* value[]) {
    struct FMU3_CSME_RTWCG * fmustruct = (struct FMU3_CSME_RTWCG *)(*fmuv);
    fmi3Status fmi3Flag = fmustruct->setString(fmustruct->mFMIComp, vr, nvr, (const fmi3String*)value ,nvr);
    return CheckStatus(fmustruct, fmi3Flag, "fmi3SetString");
}

fmi3Boolean FMU3_getFloat64(void **fmuv,
                        const fmi3ValueReference vr[],
                        size_t nvr,
                        fmi3Float64 value[], size_t nVal) {
    struct FMU3_CSME_RTWCG * fmustruct = (struct FMU3_CSME_RTWCG *)(*fmuv);
    fmi3Status fmi3Flag = fmustruct->getFloat64(fmustruct->mFMIComp, vr, nvr, value, nVal);
    return CheckStatus(fmustruct, fmi3Flag, "fmi3GetFloat64");
}

fmi3Boolean FMU3_getFloat32(void **fmuv,
                        const fmi3ValueReference vr[],
                        size_t nvr,
                        fmi3Float32 value[],
                        size_t nVal) {
    struct FMU3_CSME_RTWCG * fmustruct = (struct FMU3_CSME_RTWCG *)(*fmuv);
    fmi3Status fmi3Flag = fmustruct->getFloat32(fmustruct->mFMIComp, vr, nvr, value, nVal);
    return CheckStatus(fmustruct, fmi3Flag, "fmi3GetFloat32");
}

fmi3Boolean FMU3_getInt8(void **fmuv,
                        const fmi3ValueReference vr[],
                        size_t nvr,
                        fmi3Int8 value[],
                        size_t nVal) {
    struct FMU3_CSME_RTWCG * fmustruct = (struct FMU3_CSME_RTWCG *)(*fmuv);
    fmi3Status fmi3Flag = fmustruct->getInt8(fmustruct->mFMIComp, vr, nvr, value, nVal);
    return CheckStatus(fmustruct, fmi3Flag, "fmi3GetInt8");
}

fmi3Boolean FMU3_getUInt8(void **fmuv,
                        const fmi3ValueReference vr[],
                        size_t nvr,
                        fmi3UInt8 value[],
                        size_t nVal) {
    struct FMU3_CSME_RTWCG * fmustruct = (struct FMU3_CSME_RTWCG *)(*fmuv);
    fmi3Status fmi3Flag = fmustruct->getUInt8(fmustruct->mFMIComp, vr, nvr, value, nVal);
    return CheckStatus(fmustruct, fmi3Flag, "fmi3GetUInt8");
}

fmi3Boolean FMU3_getInt16(void **fmuv,
                        const fmi3ValueReference vr[],
                        size_t nvr,
                        fmi3Int16 value[],
                        size_t nVal) {
    struct FMU3_CSME_RTWCG * fmustruct = (struct FMU3_CSME_RTWCG *)(*fmuv);
    fmi3Status fmi3Flag = fmustruct->getInt16(fmustruct->mFMIComp, vr, nvr, value, nVal);
    return CheckStatus(fmustruct, fmi3Flag, "fmi3GetInt16");
}

fmi3Boolean FMU3_getUInt16(void **fmuv,
                        const fmi3ValueReference vr[],
                        size_t nvr,
                        fmi3UInt16 value[],
                        size_t nVal) {
    struct FMU3_CSME_RTWCG * fmustruct = (struct FMU3_CSME_RTWCG *)(*fmuv);
    fmi3Status fmi3Flag = fmustruct->getUInt16(fmustruct->mFMIComp, vr, nvr, value, nVal);
    return CheckStatus(fmustruct, fmi3Flag, "fmi3GetUInt16");
}

fmi3Boolean FMU3_getInt32(void **fmuv,
                        const fmi3ValueReference vr[],
                        size_t nvr,
                        fmi3Int32 value[],
                        size_t nVal) {
    struct FMU3_CSME_RTWCG * fmustruct = (struct FMU3_CSME_RTWCG *)(*fmuv);
    fmi3Status fmi3Flag = fmustruct->getInt32(fmustruct->mFMIComp, vr, nvr, value, nVal);
    return CheckStatus(fmustruct, fmi3Flag, "fmi3GetInt32");
}

fmi3Boolean FMU3_getUInt32(void **fmuv,
                        const fmi3ValueReference vr[],
                        size_t nvr,
                        fmi3UInt32 value[],
                        size_t nVal) {
    struct FMU3_CSME_RTWCG * fmustruct = (struct FMU3_CSME_RTWCG *)(*fmuv);
    fmi3Status fmi3Flag = fmustruct->getUInt32(fmustruct->mFMIComp, vr, nvr, value, nVal);
    return CheckStatus(fmustruct, fmi3Flag, "fmi3GetUInt32");
}

fmi3Boolean FMU3_getInt64Enum(void **fmuv,
                        const fmi3ValueReference vr[],
                        size_t nvr,
                        void* value,
                        size_t nVal) {
    struct FMU3_CSME_RTWCG * fmustruct = (struct FMU3_CSME_RTWCG *)(*fmuv);
    fmi3Status fmi3Flag = fmustruct->getInt64(fmustruct->mFMIComp, vr, nvr, (fmi3Int64*)value, nVal);
    return CheckStatus(fmustruct, fmi3Flag, "fmi3GetInt64");
}

fmi3Boolean FMU3_getUInt64(void **fmuv,
                        const fmi3ValueReference vr[],
                        size_t nvr,
                        void* value,
                        size_t nVal) {
    struct FMU3_CSME_RTWCG * fmustruct = (struct FMU3_CSME_RTWCG *)(*fmuv);
    fmi3Status fmi3Flag = fmustruct->getUInt64(fmustruct->mFMIComp, vr, nvr, (fmi3UInt64*)value, nVal);
    return CheckStatus(fmustruct, fmi3Flag, "fmi3GetUInt64");
}

fmi3Boolean FMU3_getBoolean(void **fmuv,
                           const fmi3ValueReference vr[],
                           size_t nvr,
                           fmi3UInt8 value[],
                           size_t nVal) {
    struct FMU3_CSME_RTWCG * fmustruct = (struct FMU3_CSME_RTWCG *)(*fmuv);
    fmi3Status fmi3Flag = fmustruct->getBoolean(fmustruct->mFMIComp, vr, nvr, (fmi3Boolean*)value ,nVal);
    return CheckStatus(fmustruct, fmi3Flag, "fmi3GetBoolean");
}

fmi3Boolean FMU3_getString(void **fmuv,
                          const fmi3ValueReference vr[],
                          size_t nvr,
                          void* value[]) {
    struct FMU3_CSME_RTWCG * fmustruct = (struct FMU3_CSME_RTWCG *)(*fmuv);
    fmi3Status fmi3Flag = fmustruct->getString(fmustruct->mFMIComp, vr, nvr, (fmi3String*)value ,nvr);
    return CheckStatus(fmustruct, fmi3Flag, "fmi3GetString");
}

void FMU3_strncpy(char* dest, void* src, int size) {
    strncpy(dest, (char*)src, size);
    dest[size] = '\x00';
}

char* FMU3_strndup(const char* src, int size) {
    size_t srclen = strlen(src);
    size_t copylen = srclen < size? srclen : size;
    char *dest= (char*)malloc(copylen+1);
    if (dest != NULL) {
        FMU3_strncpy(dest, (void*)src, copylen);
    }
    return dest;
}

fmi3Boolean FMU3_enterStepMode(void** fmuv) {
    struct FMU3_CSME_RTWCG * fmustruct = (struct FMU3_CSME_RTWCG *)(*fmuv);
    fmi3Status fmi3Flag = fmustruct->enterStepMode(fmustruct->mFMIComp);
    return CheckStatus(fmustruct, fmi3Flag, "fmi3EnterStepMode");
}

/*me standard functions wrapper*/
fmi3Boolean FMU3_enterEventMode(void** fmuv) {
    struct FMU3_CSME_RTWCG * fmustruct = (struct FMU3_CSME_RTWCG *)(*fmuv);
    fmi3Status fmi3Flag = fmustruct->enterEventMode(fmustruct->mFMIComp);
    return CheckStatus(fmustruct, fmi3Flag, "fmi3EnterEventMode");
}

/* this function was directly called in eventIteration, no wrapper needed*/
/* fmi3Status FMU3_newDiscreteStates(void** fmuv, fmi3EventInfo* eventInfo); */

fmi3Boolean FMU3_enterContinuousTimeMode(void** fmuv) {
    struct FMU3_CSME_RTWCG * fmustruct = (struct FMU3_CSME_RTWCG *)(*fmuv);
    fmi3Status fmi3Flag = fmustruct->enterContinuousTimeMode(fmustruct->mFMIComp);
    return CheckStatus(fmustruct, fmi3Flag, "fmi3EnterContinuousTimeMode");
}

fmi3Boolean FMU3_completedIntegratorStep(void** fmuv,
                                        fmi3Boolean noSetFMUStatePriorToCurrentPoint,
                                        uint8_t* enterEventMode,
                                        uint8_t* terminateSimulation) {
    struct FMU3_CSME_RTWCG * fmustruct = (struct FMU3_CSME_RTWCG *)(*fmuv);
    fmi3Status fmi3Flag = fmustruct->completedIntegratorStep(fmustruct->mFMIComp, noSetFMUStatePriorToCurrentPoint, (fmi3Boolean*)enterEventMode, (fmi3Boolean*)terminateSimulation);
    return CheckStatus(fmustruct, fmi3Flag, "fmi3CompletedIntegratorStep");

}

fmi3Boolean FMU3_setTime(void** fmuv,
                        fmi3Float64 time) {
    struct FMU3_CSME_RTWCG * fmustruct = (struct FMU3_CSME_RTWCG *)(*fmuv);
    fmi3Status fmi3Flag = fmustruct->setTime(fmustruct->mFMIComp, time);
    return CheckStatus(fmustruct, fmi3Flag, "fmi3SetTime");
}

fmi3Boolean FMU3_setContinuousStates(void** fmuv,
                                    const fmi3Float64 states[],
                                    size_t nx)  {
    struct FMU3_CSME_RTWCG * fmustruct = (struct FMU3_CSME_RTWCG *)(*fmuv);
    fmi3Status fmi3Flag = fmustruct->setContinuousStates(fmustruct->mFMIComp, states, nx);
    return CheckStatus(fmustruct, fmi3Flag, "fmi3SetContinuousStates");
}

fmi3Boolean FMU3_getDerivatives(void** fmuv,
                               fmi3Float64 derivatives[],
                               size_t nx) {
    struct FMU3_CSME_RTWCG * fmustruct = (struct FMU3_CSME_RTWCG *)(*fmuv);
    // FIXME fmu3codegen_tag check if this is the correct function
    fmi3Status fmi3Flag = fmustruct->getContinuousStateDerivatives(fmustruct->mFMIComp, derivatives, nx);
    return CheckStatus(fmustruct, fmi3Flag, "fmi3GetDerivatives");
}

fmi3Boolean FMU3_getEventIndicators(void** fmuv,
                                   fmi3Float64 eventIndicators[],
                                   size_t nx) {
    struct FMU3_CSME_RTWCG * fmustruct = (struct FMU3_CSME_RTWCG *)(*fmuv);
    fmi3Status fmi3Flag = fmustruct->getEventIndicators(fmustruct->mFMIComp, eventIndicators, nx);
    return CheckStatus(fmustruct, fmi3Flag, "fmi3GetEventIndicators");
}

fmi3Boolean FMU3_getContinuousStates(void** fmuv,
                                    fmi3Float64 states[],
                                    size_t nx)  {
    struct FMU3_CSME_RTWCG * fmustruct = (struct FMU3_CSME_RTWCG *)(*fmuv);
    fmi3Status fmi3Flag = fmustruct->getContinuousStates(fmustruct->mFMIComp, states, nx);
    return CheckStatus(fmustruct, fmi3Flag, "fmi3GetContinuousStates");
}

fmi3Boolean FMU3_getNominalsOfContinuousStates(void** fmuv,
                                              fmi3Float64 states[],
                                              size_t nx) {
    struct FMU3_CSME_RTWCG * fmustruct = (struct FMU3_CSME_RTWCG *)(*fmuv);
    fmi3Status fmi3Flag = fmustruct->getNominalsOfContinuousStates(fmustruct->mFMIComp, states, nx);
    return CheckStatus(fmustruct, fmi3Flag, "fmi3GetNominalsOfContinuousStates");
}

/* me helper functions*/
void FMU3_getNextEventTime(void **fmuv,
                           fmi3Float64* nextEventTime,
                           int32_T* upcomingTimeEvent) {
    struct FMU3_CSME_RTWCG * fmustruct = (struct FMU3_CSME_RTWCG *)(*fmuv);
    *nextEventTime = fmustruct->nextEventTime;
    *upcomingTimeEvent = (int32_T) fmustruct->nextEventTimeDefined;
}

void FMU3_simTerminate(void **fmuv,
                       const char* blkPath,
                       fmi3Float64 time){
    struct FMU3_CSME_RTWCG* fmustruct = (struct FMU3_CSME_RTWCG *)(*fmuv);
#if FMU_CG_TARGET < FMUCG_STANDALONE_TARGETS
    /* terminate the simulation (successfully) */
    if (fmustruct->isLoggingOn) {
        void * diagnostic = CreateDiagnosticAsVoidPtr("FMUBlock:FMU3:FMU3SimEventUpdateTerminated", 2,
                                                      CODEGEN_SUPPORT_ARG_STRING_TYPE, blkPath,
                                                      CODEGEN_SUPPORT_ARG_REAL_TYPE, time);
        rt_ssReportDiagnosticAsInfo(fmustruct->ssPtr, diagnostic);
    }
    ssSetStopRequested(fmustruct->ssPtr, 1);
#else
    // FIXME fmu3codegen_tag
    UNUSED_PARAMETER(fmustruct);
    UNUSED_PARAMETER(time);
    UNUSED_PARAMETER(blkPath);
    fmi3String message = formatString("'TerminateSimulation' flag was set to true by FMU '%s' during event iteration. Simulation will stop after the current time step at '%f'.", blkPath, time);
    fmustruct->logMessage(fmustruct->instanceEnvironment, fmustruct->FMUErrorStatus, "FMU3_simTerminate", message);
#endif
}

fmi3Boolean FMU3_eventIteration(void **fmuv,
                         const char* blkPath,
                         fmi3Float64 time) {
    struct FMU3_CSME_RTWCG* fmustruct = (struct FMU3_CSME_RTWCG *)(*fmuv);
    fmi3Status fmi3Flag = fmi3OK;
    fmi3Boolean returnStatus = fmi3True;
    int iterationNumber = 0;
    fmi3String message = NULL;

#if FMU_CG_TARGET < FMUCG_STANDALONE_TARGETS
    (void) message;
#endif
    fmustruct->newDiscreteStatesNeeded = fmi3True;
    while(fmustruct->newDiscreteStatesNeeded == fmi3True){
        /*safe call to newDiscreteStates*/
        fmi3Flag = fmustruct->updateDiscreteStates(fmustruct->mFMIComp, &fmustruct->newDiscreteStatesNeeded,
                                                                        &fmustruct->terminateSimulation,
                                                                        &fmustruct->nominalsOfContinuousStatesChanged,
                                                                        &fmustruct->valuesOfContinuousStatesChanged,
                                                                        &fmustruct->nextEventTimeDefined,
                                                                        &fmustruct->nextEventTime);
        returnStatus = CheckStatus(fmustruct, fmi3Flag, "fmi3NewDiscreteStates");

        if(returnStatus == fmi3True && fmustruct->terminateSimulation == fmi3True){
            /* terminate the simulation (successfully) */
            FMU3_simTerminate(fmuv, blkPath, time);
        }

        if(iterationNumber >= 10000){
#if FMU_CG_TARGET < FMUCG_STANDALONE_TARGETS
            if (fmustruct->isLoggingOn) {
                void * diagnostic = CreateDiagnosticAsVoidPtr("FMUBlock:FMU:FMUSimEventUpdateTerminated", 2,
                                                              CODEGEN_SUPPORT_ARG_REAL_TYPE, time,
                                                              CODEGEN_SUPPORT_ARG_INTEGER_TYPE, iterationNumber);
                rt_ssReportDiagnosticAsInfo(fmustruct->ssPtr, diagnostic);
            }
#else
            message = formatString("'TerminateSimulation' flag was set to true by FMU '%s' during event iteration. Simulation will stop after the current time step at '%f'.", blkPath, time);
            fmustruct->logMessage(fmustruct->instanceEnvironment, fmustruct->FMUErrorStatus, "FMU3_eventIteration", message);
#endif
            returnStatus = fmi3False;
            break;

        } else
            iterationNumber ++;
    }
    return returnStatus;
}

void FMU3_valuesOfContinuousStatesChanged(void **fmuv,
                                          int* stateChanged) {
    struct FMU3_CSME_RTWCG* fmustruct = (struct FMU3_CSME_RTWCG *)(*fmuv);
    *stateChanged = fmustruct->valuesOfContinuousStatesChanged;
}

/*helper to preprocess Enum type*/
void FMU3_CreateParamIdxToOffset(void** fmuv,
                            int array_size) {
    struct FMU3_CSME_RTWCG * fmustruct = (struct FMU3_CSME_RTWCG *)(*fmuv);
    fmustruct->paramIdxToOffset = calloc(array_size, sizeof(int));
}

void FMU3_CreateEnumValueList(void** fmuv,
                         int array_size) {
    struct FMU3_CSME_RTWCG * fmustruct = (struct FMU3_CSME_RTWCG *)(*fmuv);
    fmustruct->enumValueList = calloc(array_size, sizeof(int));
}

void FMU3_SetParamIdxToOffsetByIdx(void** fmuv,
                              int idx,
                              int value) {
    struct FMU3_CSME_RTWCG * fmustruct = (struct FMU3_CSME_RTWCG *)(*fmuv);
    *(fmustruct->paramIdxToOffset + idx) = value;
}

void FMU3_SetEnumValueListByIdx(void** fmuv,
                           int idx,
                           int value) {
    struct FMU3_CSME_RTWCG * fmustruct = (struct FMU3_CSME_RTWCG *)(*fmuv);
    *(fmustruct->enumValueList + idx) = value;
}

void FMU3_GetParamIdxToOffsetByIdx(void** fmuv,
                              int idx,
                              int* offset) {
    struct FMU3_CSME_RTWCG * fmustruct = (struct FMU3_CSME_RTWCG *)(*fmuv);
    *offset = *(fmustruct->paramIdxToOffset + idx);
}

void FMU3_GetEnumValueByIdx(void** fmuv,
                       int idx,
                       int* val) {
    struct FMU3_CSME_RTWCG * fmustruct = (struct FMU3_CSME_RTWCG *)(*fmuv);
    *val = *(fmustruct->enumValueList + idx);
}


/*================================================================*

 * Functions to support LinIdx for Row-major and Col-major arrays *

 *================================================================*/

static void incrementIdxVectorColumnMajor( uint32_t *idxVector,  const uint32_t *inDim, const int  nDim) {
    int  i;
    if (!idxVector || !inDim)
        return;
    i = 0;
    while (i < nDim) {
        idxVector[i]++;
        if (idxVector[i] >= inDim[i] ) { idxVector[i++] = 0; }
        else { break; }
    }
}

static void incrementIdxVectorRowMajor( uint32_t *idxVector,  const uint32_t *inDim, const int  nDim) {
    int  i;
    if (!idxVector || !inDim)
        return;
    i = nDim-1;
    while (i>= 0) {
        idxVector[i]++;
        if (idxVector[i] >= inDim[i] ) { idxVector[i--] = 0; }
        else { break; }
    }

}


static int getColMajorLinIdxFromIdxVec(const uint32_t *idxVector, const uint32_t *inDim, const int nDim){
    int linIdx, stride, i;
    if (!idxVector || !inDim)
        return -1;

    linIdx=0;
    stride=1;
    for (i=0; i<nDim; i++){
         linIdx += stride*idxVector[i];
         stride *= inDim[i];
    }
    return linIdx;

}

static int getRowMajorLinIdxFromIdxVec(const uint32_t *idxVector, const uint32_t *inDim, const int nDim){
    int linIdx, stride, i;
    if (!idxVector || !inDim)
        return -1;

    int numOfTotalElements = 1;
    for (i=0; i<nDim; i++)
         numOfTotalElements *= inDim[i];

    linIdx=0;
    stride = numOfTotalElements;
    for (i=0; i<nDim; i++)
    {
        stride /= inDim[i];
        linIdx += stride*idxVector[i];
    }
    return linIdx;
}

void FMU3_CompareInputWithWorkingVec(void *dstWorkingVec, const void* srcBlkInput, const uint32_t* dimList, const size_t nVal, const size_t datasize, bool needConversion, int32_t *is_diff_value) {
    uint32_t idxVector[64]; // using 64 as the max number of dimensions.
    int nDim, linIdx, j;
    nDim = dimList[0]; // Skipping, max dims check
    const uint32_t *inDim = &dimList[1]; // Skipping, max dims check

    if(!dstWorkingVec || !srcBlkInput || !dimList) {
        return;
    }
    // initialize index vector
    for (j = 0; j < nDim; j++){
        idxVector[j] = 0;
    }

    if(!needConversion) {
        *is_diff_value = *is_diff_value | (memcmp(dstWorkingVec, srcBlkInput, nVal*datasize) !=0 );
        return;
    }

    for (j = 0; j < nVal; j++)
    {
        linIdx = getColMajorLinIdxFromIdxVec(idxVector, inDim, nDim);
        *is_diff_value = *is_diff_value | (memcmp(((char*)dstWorkingVec + (j*datasize)), ((char*)srcBlkInput + (linIdx*datasize)), datasize) != 0);
        incrementIdxVectorRowMajor(idxVector, inDim, nDim);
    }
}

void FMU3_UpdateVectorForFmu(void *dstWorkingVec, const void* srcBlkInput, const uint32_t* dimList, const size_t nVal, const size_t datasize, bool needConversion) {
    uint32_t idxVector[64]; // using 64 as the max number of dimensions.
    int nDim, linIdx, j;
    nDim = dimList[0]; // Skipping, max dims check
    const uint32_t *inDim = &dimList[1]; // Skipping, max dims check
    
    if(!dstWorkingVec || !srcBlkInput || !dimList) {
        return;
    }
    // initialize index vector
    for (j = 0; j < nDim; j++){
        idxVector[j] = 0;
    }

    if(!needConversion) {
        memcpy(dstWorkingVec, srcBlkInput, nVal*datasize);
        return;
    }

    for (j = 0; j < nVal; j++)
    {
        linIdx = getColMajorLinIdxFromIdxVec(idxVector, inDim, nDim);
        memcpy(((char*)dstWorkingVec + (j*datasize)), ((char*)srcBlkInput + (linIdx*datasize)), datasize);
        incrementIdxVectorRowMajor(idxVector, inDim, nDim);
    }
}

void FMU3_UpdateVectorForBlk(void *dstBlkOutput, const void* srcWorkingVector, const uint32_t* dimList, const size_t nVal, const size_t datasize, bool needConversion) {
    uint32_t idxVector[64]; // using 64 as the max number of dimensions.
    int nDim, linIdx, j;
    nDim = dimList[0]; // Skipping, max dims check
    const uint32_t *inDim = &dimList[1]; // Skipping, max dims check

    if(!dstBlkOutput || !srcWorkingVector || !dimList) {
        return;
    }
    // initialize index vector
    for (j = 0; j < nDim; j++){
        idxVector[j] = 0;
    }

    if(!needConversion) {
        memcpy(dstBlkOutput, srcWorkingVector, nVal*datasize);
        return;
    }

    for (j = 0; j < nVal; j++)
    {
        linIdx = getRowMajorLinIdxFromIdxVec(idxVector, inDim, nDim);
        memcpy(((char*)dstBlkOutput + (j*datasize)), ((char*)srcWorkingVector + (linIdx*datasize)), datasize);
        incrementIdxVectorColumnMajor(idxVector, inDim, nDim);
    }
}
