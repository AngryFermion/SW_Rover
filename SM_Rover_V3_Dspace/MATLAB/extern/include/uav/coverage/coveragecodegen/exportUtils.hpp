/* Copyright 2022-2023 The MathWorks, Inc. */
#ifndef LIBMWCOVERAGE_UTIL_HPP
#define LIBMWCOVERAGE_UTIL_HPP

#if defined(BUILDING_LIBMWCOVERAGECODEGEN)
/* For DLL_EXPORT_SYM and EXTERN_C */
#include "package.h"
/* For uint32_T, boolean_T, etc */
#include "tmwtypes.h"

#define LIBMWCOVERAGE_API DLL_EXPORT_SYM

#else

#if defined(MATLAB_MEX_FILE) || defined(BUILDING_UNITTEST) || defined(MATLAB_BUILTINS)
#include "tmwtypes.h"
#else
#include "rtwtypes.h"
#endif
#ifndef LIBMWCOVERAGE_API
#define LIBMWCOVERAGE_API
#endif
#endif

#ifndef EXTERN_C
#ifdef __cplusplus
#define EXTERN_C extern "C"
#else
#define EXTERN_C
#endif
#endif

#endif // LIBMWCOVERAGE_UTIL_HPP
