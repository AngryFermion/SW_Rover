/* Copyright 2024 The MathWorks, Inc. */

#ifndef logging2_sdi_stream_SscInstRtwLogging_h
#define logging2_sdi_stream_SscInstRtwLogging_h


typedef struct SscInstRtwLoggingTag SscInstRtwLogging;

/*
 * fwd declaration for the struct that is intended to contain a pointer to the client side
 * InstrumentationRtwLogging object. The client side InstrumentationRtwLogging class provides the
 * function for the C struct to interact with. SscInstRtwLogging struct is part of the published
 * code (matlab/extern/include/physmod/.....)
 */
typedef struct SscInstRtwLoggingObjectTag SscInstRtwLoggingObject;

/*!
  SscInstRtwLogging is a C interface to interact with the logging infrastructure from the generated
  code. SscInstRtwLogging is created through ssc_rtw_make_logging_struct function.

  Refer to InstrumentationRtwLogging.hpp
 */

struct SscInstRtwLoggingTag {
    /* Setup the logger from the block */
    void (*mStart)(SscInstRtwLogging const*);

    /* Record the data for the specified time-step in the logger using the
     * specified buffer */
    void (*mLog)(SscInstRtwLogging const*, double time, double const* buffer);

    /* Indicate that the logging is completed. */
    void (*mTerminate)(SscInstRtwLogging const*);

    /* Destructor - called from the custom deleter. Clients are not required to
     * call this function */
    void (*mDestroy)(SscInstRtwLogging*);

    /* Private data pointer to InstrumentationRtwLogging */
    SscInstRtwLoggingObject* mObject;
};

/* Create SscInstRtwLogging using InstrumentationRtwLogging instance. The width specifies the width
 * of the buffer associated with SscInstRtwLogging struct. */
extern
SscInstRtwLogging* ssc_inst_rtw_make_logging_struct(const char*        modelName,
                                                    const char*        blockPath,
                                                    const char*        metaDataFileName,
                                                    unsigned int const decimation);

/* Wrapper function for mStart */
extern
void ssc_inst_logger_start(SscInstRtwLogging const*);

/* Wrapper function for mLog */
extern
void ssc_inst_logger_log(SscInstRtwLogging const*, double time, double const* buffer);

/* Wrapper function for mTerminate */
extern
void ssc_inst_logger_terminate(SscInstRtwLogging const*);

/* Wrapper function for mDestroy */
extern
void ssc_inst_logger_destroy(SscInstRtwLogging*);

#endif
