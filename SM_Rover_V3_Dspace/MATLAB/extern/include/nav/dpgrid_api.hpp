/**
 * @file dpgrid_api.hpp
 * @brief This file contains declarations of external C-API interfaces for DPGrid.
 */

/* Copyright 2024 The MathWorks, Inc. */

#ifndef DPGRID_CODEGEN_API_HPP
#define DPGRID_CODEGEN_API_HPP

#ifdef BUILDING_LIBMWDPGRIDCODEGEN
#include "dpgridcodegen/dpgrid_codegen_util.hpp"
#else
/* To deal with the fact that PackNGo has no include file hierarchy during test */
#include "dpgrid_codegen_util.hpp"
#endif

/**
 * @brief DPGrid constructor
 *
 */
EXTERN_C DPGRID_CODEGEN_API void* dpgrid_construct(const boolean_T* mapMatrix,
                                                   const uint32_T rows,
                                                   const uint32_T cols);

/**
 * @brief DPGrid destructor
 *
 * @param[in] dpObj DPGrid object
 */
EXTERN_C DPGRID_CODEGEN_API void dpgrid_destruct(void* dpObj);


/**
 * @brief Set goal location
 *
 * @param[in] dpObj DPGrid object
 * @param[in] goal Goal coordinates in grid frame converted to zero-indexing
 */
EXTERN_C DPGRID_CODEGEN_API void dpgrid_setGoal(void* dpObj, const uint32_T* goal);


/**
 * @brief Get path cost from a specified start location
 *
 * @param[in] dpObj DPGrid object
 * @param[in] start Start coordinates in grid frame converted to zero-indexing
 * @return cost Path cost from specified start to goal
 */
EXTERN_C DPGRID_CODEGEN_API real64_T dpgrid_getPathCost(void* dpObj, const uint32_T* start);


#endif
