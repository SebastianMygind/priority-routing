#pragma once
#include "Graph.h"

class IPathFinder
{
public:
    virtual ~IPathFinder() = default;

    virtual bool FindPath(
        Graph& graph,
        uint64_t start_node,
        uint64_t end_node,
        std::vector<uint64_t>& out_path) = 0;
};

enum class PathfindingModel {
    Dijkstra = 0,
    AStar = 1
};

void PathFinder(Graph& graph, PathfindingModel model);