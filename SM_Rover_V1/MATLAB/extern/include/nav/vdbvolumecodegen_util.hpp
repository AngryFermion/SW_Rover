/* Copyright 2023-2024 The MathWorks, Inc. */
#ifndef VDBVOLUMECODEGEN_SPEC_HPP
#define VDBVOLUMECODEGEN_SPEC_HPP

#ifdef BUILDING_LIBMWVDBVOLUMECODEGEN

/* For DLL_EXPORT_SYM and EXTERN_C */
#include "package.h"
/* For uint32_T, boolean_T, etc*/
#include "tmwtypes.h"

#define VDBMANAGER_EXPORT_API DLL_EXPORT_SYM

#else

/* For uint32_T, boolean_T, etc */
/* Consuming MATLAB C++ module should define MATLAB_BUILTINS token in its makefile */
#if defined(MATLAB_MEX_FILE) || defined(BUILDING_UNITTEST) || defined(MATLAB_BUILTINS)
#include "tmwtypes.h"
#else
#include "rtwtypes.h"
#endif

#ifndef EXTERN_C
#ifdef __cplusplus
#define EXTERN_C extern "C"
#else
#define EXTERN_C extern
#endif
#endif

#ifndef VDBMANAGER_EXPORT_API
#define VDBMANAGER_EXPORT_API
#endif

#endif // BUILDING_LIBMWVDBVOLUMECODEGEN

#endif /* VDBVOLUMECODEGEN_SPEC_HPP */
