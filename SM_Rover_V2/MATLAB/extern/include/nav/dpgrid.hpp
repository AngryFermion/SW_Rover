/**
 * @file dpgrid.hpp
 * @brief This file contains the declarations for DPGrid
 */

// Copyright 2024 The MathWorks, Inc.
#ifndef DP_GRID_HPP
#define DP_GRID_HPP

#include <iostream>
#include <vector>
#include <array>
#include <queue>
#include <climits>
#include <algorithm>
#include <cmath>
#include <limits>

#ifdef BUILDING_LIBMWDPGRIDCODEGEN
#include "dpgridcodegen/dpgrid_codegen_util.hpp"
#else
/* To deal with the fact that PackNGo has no include file hierarchy during test */
#include "dpgrid_codegen_util.hpp"
#endif

namespace nav {
/**
 * @brief DPGrid algorithm that finds path between start and goal.
 *
 * Does backwards search from the goal to start and memoize the
 * path costs for the nodes that are explored during the search. This
 * is especially useful for computing the 2D obstacle heuristic for
 * Hybrid A* algorithm.
 *
 */
class DPGrid {
  private:
    // Occupancy matrix of the input map
    std::vector<std::vector<bool>> _mapMatrix;

    // Number of rows and columns in the map
    uint32_T _rows;
    uint32_T _cols;

    // Start grid location [row, col] in zero-index format
    std::array<uint32_t, 2> _start = {0, 0};

    // Goal grid location [row, col] in zero-index format
    std::array<uint32_t, 2> _goal = {0, 0};

    // Cost to goal matrix between from a grid cell in the map to the goal cell
    // = inf means the cell is already occupied
    // = 0 means the cell is initialized but not processed yet
    // < 0 means the cell is processed but the cost has not converged yet
    // > 0 means the cell cost has converged and won't be further processed
    std::vector<std::vector<double>> _costToGoal;

    // Store grid cell data for the open set. We store the location and the cost estimate
    struct _cellData {
        std::array<uint32_t, 2> location;
        double cost;
        _cellData(std::array<uint32_t, 2> loc, double costval)
            : location(loc)
            , cost(costval) {}
    };

    // Priority queue for ordering the grid cells explored
    // First element contains the cost
    // Second element contains [row, col] pair
    std::vector<_cellData> _priorityQueue;

    // Min heap comparator for the priority queue
    // Sorts the _priorityQueue such that smallest element is at the front
    struct _minHeapComparator {
        bool operator()(const _cellData& a, const _cellData& b) const {
            return a.cost > b.cost;
        }
    };

    // Grid surrounding directions - south, north, west, east and diagonal directions
    const std::vector<std::array<int8_t, 2>> _directions = {{-1, 0},  {1, 0},  {0, -1}, {0, 1},
                                                            {-1, -1}, {-1, 1}, {1, -1}, {1, 1}};


    // Function to initialize cost matrix
    void initializeCostMatrix();

    // Function to find valid neighbors for a given grid cell
    std::vector<std::array<uint32_T, 2>> getValidNeighbors(const std::array<uint32_T, 2>& current);

    // Transition cost function from a cell to its neighbors
    double transitionCost(const std::array<uint32_T, 2>& currentCell,
                          const std::array<uint32_T, 2>& nextCell);

    // Distance between any two cells in the map
    double distance2D(const std::array<uint32_T, 2>& cellI, const std::array<uint32_T, 2>& cellJ);

    // Get next cell based on current cell, i (unsigned) and
    // direction, dir (can be positive or negative)
    uint32_T nextCell(uint32_T i, int dir);

  public:
  
    DPGrid(const std::vector<std::vector<bool>>& mapMatrix);

    /**
     * @brief Set start location
     *
     * @param[in] start Start coordinates in grid frame converted to zero-indexing
     */
    void setStart(const uint32_T* start);

    /**
     * @brief Set goal location
     *
     * @param[in] goal Goal coordinates in grid frame converted to zero-indexing
     */
    void setGoal(const uint32_T* goal);

    /**
     * @brief Plan path between specified start and goal
     */
    void plan();

    /**
     * @brief Get path cost from a specified start location
     * @param[in] start Start coordinates in grid frame converted to zero-indexing
     * @return cost Path cost from specified start to goal
     */
    double getPathCost(const uint32_T* start);
};

} // namespace nav

#endif // DP_GRID_HPP
