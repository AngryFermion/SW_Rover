/* Copyright 2008-2022 The MathWorks, Inc. */
#ifndef COSIM_SYSINTERF_RTW_SPEC_HPP
#define COSIM_SYSINTERF_RTW_SPEC_HPP

#ifdef SUPPORTS_PRAGMA_ONCE
#pragma once
#endif

#ifdef BUILDING_COSIM_SYSINTERF_RTW

/* This header is being included by files inside this module */
#include "package.h"
#define COSIM_SYSINTERF_RTW_EXPORT_CLASS DLL_EXPORT_SYM
#define COSIM_SYSINTERF_RTW_EXPORT_EXTERN_C extern "C" DLL_EXPORT_SYM

#elif defined(DLL_IMPORT_SYM)

/* This file is being include by other TMW internal modules */
#ifdef __cplusplus
#define COSIM_SYSINTERF_RTW_EXPORT_CLASS DLL_IMPORT_SYM
#define COSIM_SYSINTERF_RTW_EXPORT_EXTERN_C extern "C" DLL_IMPORT_SYM
#else
#define COSIM_SYSINTERF_RTW_EXPORT_EXTERN_C extern DLL_IMPORT_SYM
#endif

#else

/* This file is being include by other TMW internal modules */
#if defined(__cplusplus)
#define COSIM_SYSINTERF_RTW_EXPORT_EXTERN_C extern "C"
#else
#define COSIM_SYSINTERF_RTW_EXPORT_EXTERN_C extern
#endif

#endif

#endif
