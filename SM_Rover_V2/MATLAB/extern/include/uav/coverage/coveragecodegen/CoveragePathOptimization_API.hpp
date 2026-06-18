/* Copyright 2022-2023 The MathWorks, Inc. */
#pragma once
#ifndef COVERAGEOPTIM_API_HPP_
#define COVERAGEOPTIM_API_HPP_

#ifdef BUILDING_LIBMWCOVERAGECODEGEN
#include "coveragecodegen/exportUtils.hpp"
#else
/* To deal with the fact that PackNGo has no include file hierarchy during test */
#include "exportUtils.hpp"
#endif
// unlike the C++ underlying call. Vectors cannot be input to C interface.
/// Exhaustive Planner API plan coverage path using Exhaustive Path Traversal.
/**
* @param[in] VisitOrderSpecified  flag indicating if Visit Order of Polygons is specified.
* @param[out] SpecifiedVisitSequenceC a pointer to the Visiting order.
* @param[in] MinAdjacencyCount  Minimum Adjacency Count required for permutation to be considered
valid.
* @param[in] StartPosesC  row vector containing start positions corresponding to four different sweep
patterns in each polygon.
* @param[out] EndPosesC row vector containing exit positions corresponding to four different sweep
patterns in each polygon.
* @param[in] TakeoffArr 1x3 array containing information about Takeoff location.
* @param[in] LandingArr 1x3 array containing information about Landing location.
* @param[in] NumPolygons Number of polygons that guides parsing of sequence.
* @param[in] NumPoses Number of start and end poses.
* @param[in] AdjacencyInfoC row vector containing adjacency of all polygons.
* @param[in] NumAdjacency number of adjacency.
* @param[out] SequenceC  Sequence of visiting polygons.
* @param[out] SweepSequenceC  Sweep sequence of visiting polygons.
* @param[out] TransitionCost  Cost to transition between polygons for given sequence and sweep
sequence.

*/
// Exhaustive Planner API
EXTERN_C LIBMWCOVERAGE_API void planExhaustivePathAPI(size_t* SequenceC,
                                                      size_t* SweepSequenceC,
                                                      double* TransitionCost, // Outputs
                                                      bool VisitOrderSpecified,
                                                      size_t const* SpecifiedVisitSequenceC,
                                                      int MinAdjacencyCount, // Unique Inputs
                                                      size_t const* AdjacencyInfoC,
                                                      double const* StartPosesC,
                                                      double const* EndPosesC,
                                                      double* TakeoffArr,
                                                      double* LandingArr,
                                                      size_t NumPolygons,
                                                      size_t NumPoses,
                                                      size_t NumAdjacency);
/// planMinTraversalPathAPI plan coverage path using Minimum Path Traversal.
/**
* @param[in] VisitOrderSpecified  flag indicating if Visit Order of Polygons is specified.
* @param[out] SpecifiedVisitSequenceC a pointer to the Visiting order.
* @param[in] StartIndex Index of Starting Polygon to compute algorithm from.
* @param[in] StartPosesC  row vector containing start positions corresponding to four different sweep
patterns in each polygon.
* @param[out] EndPosesC row vector containing exit positions corresponding to four different sweep
patterns in each polygon.
* @param[in] TakeoffArr 1x3 array containing information about Takeoff location.
* @param[in] LandingArr 1x3 array containing information about Landing location.
* @param[in] NumPolygons Number of polygons that guides parsing of sequence.
* @param[in] NumPoses Number of start and end poses.
* @param[in] AdjacencyInfoC row vector containing adjacency of all polygons.
* @param[in] NumAdjacency number of adjacency.
* @param[out] SequenceC  Sequence of visiting polygons.
* @param[out] SweepSequenceC  Sweep sequence of visiting polygons.
* @param[out] TransitionCost  Cost to transition between polygons for given sequence and sweep
sequence.

*/
EXTERN_C LIBMWCOVERAGE_API void planMinTraversalPathAPI(size_t* SequenceC,
                                                        size_t* SweepSequenceC,
                                                        double* TransitionCost, // Outputs
                                                        bool VisitOrderSpecified,
                                                        size_t const* SpecifiedVisitSequenceC,
                                                        size_t StartIndex, // Unique Inputs
                                                        size_t const* AdjacencyInfoC,
                                                        double const* StartPosesC,
                                                        double const* EndPosesC,
                                                        double* TakeoffArr,
                                                        double* LandingArr,
                                                        size_t NumPolygons,
                                                        size_t NumPoses,
                                                        size_t NumAdjacency);
#endif

