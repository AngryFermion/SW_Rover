/* Copyright 2023 The MathWorks, Inc. */
#pragma once

#include "cosim_sysinterf_rtw_spec.h"
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

COSIM_SYSINTERF_RTW_EXPORT_EXTERN_C void registerBus(const size_t busNameLength,
                                                     const char* busName,
                                                     const uint64_t busSize);

COSIM_SYSINTERF_RTW_EXPORT_EXTERN_C void addElementInfo(const size_t busNameLength,
                                                        const char* busName,
                                                        const size_t elemNameLength,
                                                        const char* elemName,
                                                        const size_t dataTypeLength,
                                                        const char* dataType,
                                                        const size_t complexityLength,
                                                        const char* complexity,
                                                        const size_t numDims,
                                                        const uint64_t* dimensions,
                                                        const uint64_t offset);

COSIM_SYSINTERF_RTW_EXPORT_EXTERN_C bool writeToProto(const size_t busNameLength,
                                                      const char* fileName);
