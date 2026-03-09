#pragma once
#include "Graph.h"

class IPathFinder
{
public:
    virtual ~IPathFinder() = default;

    virtual bool FindPath(
        OSMGraph& graph,
        uint64_t start_node,
        uint64_t end_node,
        std::set<uint64_t>& out_path) = 0;
};

enum class PathfindingModel {
    Dijkstra = 0,
    AStar = 1
};

void PathFinder(OSMGraph& graph, PathfindingModel model);