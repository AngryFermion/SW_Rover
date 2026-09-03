#ifndef SLPROCESS_H
#define SLPROCESS_H
/**
 * @file slprocess.h
 *
 * This header is the exported C interface to the slprocess library
 *
 */

/* Copyright 2024 The MathWorks, Inc. */

#include "tmwtypes.h"

/*******************************************************************************
 * Section 1. Symbol visibility
 *******************************************************************************/
#ifdef BUILDING_LIBMWSLPROCESS_CORE
/* being included from inside the live common plugin client module */
#include "package.h"
#define SLPROCESS_PUBLISHED_C extern "C" DLL_EXPORT_SYM
#else
/* being included from outside, such as raccel code */
#if defined(__cplusplus)
/* Needed for non-inlined C++ S-functions */
#define SLPROCESS_PUBLISHED_C extern "C"
#else
#define SLPROCESS_PUBLISHED_C extern
#endif
#endif

/********************************************************************************
 * Section 2. Process Registry c-api
 *******************************************************************************/
/* create a slprocess object and returns a handle for tracking processes created
   via the capi */
SLPROCESS_PUBLISHED_C uint32_T slprocessCreateHandle();
/* removes a handle and destroys the slprocess object created via the capi */
SLPROCESS_PUBLISHED_C void slprocessRemoveHandle(uint32_T);

/********************************************************************************
 * Section 3. Process Properties c-api
 *******************************************************************************/
SLPROCESS_PUBLISHED_C bool slprocessAlive(uint32_T idx);

SLPROCESS_PUBLISHED_C int32_T slprocessPid(uint32_T idx);

SLPROCESS_PUBLISHED_C int32_T slprocessStatus(uint32_T idx);

SLPROCESS_PUBLISHED_C void slprocessResult(uint32_T idx, char* result, uint32_T size);

SLPROCESS_PUBLISHED_C double slprocessPollingPeriod(uint32_T idx);

SLPROCESS_PUBLISHED_C void slprocessSetPollingPeriod(uint32_T idx, double val);

SLPROCESS_PUBLISHED_C void slprocessSetStartDir(uint32_T idx, const char*);

SLPROCESS_PUBLISHED_C void slprocessSetEnv(uint32_T idx, const char* key, const char* val);


/********************************************************************************
 * Section 4. Process commands
 *******************************************************************************/
/* initialize and runs a proces using the command argument */
SLPROCESS_PUBLISHED_C void slprocessRun(uint32_T idx, const char* cmd);

/* initialize and runs a proces in a new thread using the command argument */
SLPROCESS_PUBLISHED_C void slprocessStart(uint32_T idx, const char* cmd);

/* wait for started process to finish */
SLPROCESS_PUBLISHED_C void slprocessWait(uint32_T idx);

/* terminates running process */
SLPROCESS_PUBLISHED_C void slprocessTerminate(uint32_T idx);

#endif
