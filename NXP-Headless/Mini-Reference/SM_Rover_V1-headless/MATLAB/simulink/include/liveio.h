#ifndef LIVEIO_H
#define LIVEIO_H
/**
 * @file slexec_simbridge.h
 *
 * This header is the exported C interface to the live IO library
 *
 */

/* Copyright 2023 The MathWorks, Inc. */

#include "tmwtypes.h"

/*******************************************************************************
 * Section A.1. Symbol visibility
 *******************************************************************************/
#ifdef BUILDING_LIBMWLIVE_COMMON_PLUGIN_CLIENT
/* being included from inside the live common plugin client module */
#include "package.h"
#define LIVE_COMMON_PLUGIN_PUBLISHED_C extern "C" DLL_EXPORT_SYM
#else
/* being included from outside, such as raccel code */
#if defined(__cplusplus)
/* Needed for non-inlined C++ S-functions */
#define LIVE_COMMON_PLUGIN_PUBLISHED_C extern "C"
#else
#define LIVE_COMMON_PLUGIN_PUBLISHED_C extern
#endif
#endif

/********************************************************************************
 * Section A.2. Live Type Registry c-api
 *******************************************************************************/
LIVE_COMMON_PLUGIN_PUBLISHED_C uint32_T liveioGetTypeId(const char* name);

LIVE_COMMON_PLUGIN_PUBLISHED_C uint32_T liveioRegisterNumericType(const char* name,
                                                                  int32_T mdArrayType,
                                                                  bool complex);

LIVE_COMMON_PLUGIN_PUBLISHED_C uint32_T liveioRegisterArrayType(const char* name,
                                                                uint32_T baseTypeId,
                                                                uint64_T* dimsPtr,
                                                                uint64_T numDims);

LIVE_COMMON_PLUGIN_PUBLISHED_C uint32_T liveioRegisterAggregateType(const char* name);

LIVE_COMMON_PLUGIN_PUBLISHED_C void liveioRegisterAggregateTypeField(const char* aggregateName,
                                                                     const char* fieldName,
                                                                     uint32_T fieldType);

LIVE_COMMON_PLUGIN_PUBLISHED_C uint32_T liveioRegisterStructType(const char* name,
                                                                 uint64_T alignment);

LIVE_COMMON_PLUGIN_PUBLISHED_C void liveioRegisterStructTypeField(uint32_T structType,
                                                                  const char* fieldName,
                                                                  uint32_T fieldType,
                                                                  void* struct_address,
                                                                  void* field_address);

LIVE_COMMON_PLUGIN_PUBLISHED_C void liveioRegisterStructTypeFieldWithOffset(uint32_T structType,
                                                                            const char* fieldName,
                                                                            uint32_T fieldType,
                                                                            uint64_T field_address);

LIVE_COMMON_PLUGIN_PUBLISHED_C void liveioUnregisterTypeId(uint32_T typeId);

LIVE_COMMON_PLUGIN_PUBLISHED_C void liveioUnregisterTypeName(const char* typeName);

/********************************************************************************
 * Section A.3. Live IO creation destruction c-apis
 *******************************************************************************/
// registers a location to associate objects and types to
LIVE_COMMON_PLUGIN_PUBLISHED_C uint32_T liveioGetLiveSvcIndex(uint32_T type,
                                                              const char* path,
                                                              const char* identifier);

// creates a live io object and registers it to a service index
// returns a connection index for the created object
LIVE_COMMON_PLUGIN_PUBLISHED_C uint32_T liveioCreateObject(uint32_T svcIndex, const char* jsonSpec);

// removes all live objects and type information associated with a location
LIVE_COMMON_PLUGIN_PUBLISHED_C bool liveioRemoveLiveSvc(uint32_T type,
                                                        const char* path,
                                                        const char* identifier);


/********************************************************************************
 * Section A.4. Live IO pubsub c-apis
 *******************************************************************************/
// synchronously publishes blob on all publishers associated to a service index
// returns true if send was successful to all publishers
LIVE_COMMON_PLUGIN_PUBLISHED_C bool liveioSvcPublish(uint32_T svcIndex,
                                                     uint64_T timeoutMs,
                                                     void const* data,
                                                     uint64_T dataSize);

// synchronously publishes a registered matlab data type on all publishers associated to a service
// index returns true if send was successful to all publishers
LIVE_COMMON_PLUGIN_PUBLISHED_C bool liveioSvcPublishData(uint32_T svcIndex,
                                                         uint64_T timeoutMs,
                                                         uint32_T srcType,
                                                         void const* data);

// synchronously publishes blob on a single publisher connection
// returns true if send was successful
LIVE_COMMON_PLUGIN_PUBLISHED_C bool liveioPublish(uint32_T connIndex,
                                                  uint64_T timeoutMs,
                                                  void const* data,
                                                  uint64_T dataSize);

// synchronously publishes a registered matlab data type on a single publisher connection
// returns true if send was successful
LIVE_COMMON_PLUGIN_PUBLISHED_C bool liveioPublishData(uint32_T connIndex,
                                                      uint64_T timeoutMs,
                                                      uint32_T srcType,
                                                      void const* data);

// takes blob from a single subscriber connection
// returns zero if no bytes were received
LIVE_COMMON_PLUGIN_PUBLISHED_C uint32_T liveioTake(uint32_T connIndex,
                                                   uint64_T timeoutMs,
                                                   void* data,
                                                   uint64_T size);

// takes data from a single subscriber connection that has a registered matlab data type
// returns zero if no bytes were received
LIVE_COMMON_PLUGIN_PUBLISHED_C uint32_T liveioTakeData(uint32_T connIndex,
                                                       uint64_T timeoutMs,
                                                       uint32_T destType,
                                                       void* data);

// returns a listener index associated with the listener object
LIVE_COMMON_PLUGIN_PUBLISHED_C uint32_T liveioCreateListener();

// register the OnLivelinessChanged callback to the listener.
LIVE_COMMON_PLUGIN_PUBLISHED_C void liveioRegisterOnLivelinessChanged(uint32_T listenerIdx);

LIVE_COMMON_PLUGIN_PUBLISHED_C void liveioRegisterOnSubscriptionChanged(uint32_T listenerIdx);

LIVE_COMMON_PLUGIN_PUBLISHED_C void liveioRegisterOnSampleLost(uint32_T listenerIdx);

// see if listener already has OnLivelinessChanged callback.
LIVE_COMMON_PLUGIN_PUBLISHED_C bool liveioHasOnLivelinessChanged(uint32_T listenerIdx);

LIVE_COMMON_PLUGIN_PUBLISHED_C bool liveioHasOnSubscriptionChanged(uint32_T listenerIdx);

LIVE_COMMON_PLUGIN_PUBLISHED_C bool liveioHasOnSampleLost(uint32_T listenerIdx);

// attaches a listener to the connection provided
LIVE_COMMON_PLUGIN_PUBLISHED_C bool liveioAttachListener(uint32_T connIdx, uint32_T listenerIdx);

// removes a listener from the connection provided
LIVE_COMMON_PLUGIN_PUBLISHED_C bool liveioRemoveListener(uint32_T connIdx);

// deletes underlying listener object given a listener index
LIVE_COMMON_PLUGIN_PUBLISHED_C void liveioDeleteListener(uint32_T listenerIdx);

// register data to be passed to callback functions
LIVE_COMMON_PLUGIN_PUBLISHED_C void liveioRegisterUserData(uint32_T listenerIdx, void* userData);


LIVE_COMMON_PLUGIN_PUBLISHED_C bool liveioIsLivelinessChanged(uint32_T listenerIdx);

LIVE_COMMON_PLUGIN_PUBLISHED_C bool liveioIsSampleLost(uint32_T listenerIdx);

LIVE_COMMON_PLUGIN_PUBLISHED_C bool liveioIsSubscriptionChanged(uint32_T listenerIdx);

#ifndef LIVEIO_FIELD_OFFSET
#define LIVEIO_FIELD_OFFSET(X, Y) (uint64_T)((uint8_T*)(&((X)->Y)) - (uint8_T*)(X))
#endif

#endif
