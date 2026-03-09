#pragma once
#include "osm/graph.h"

class IPathFinder
{
public:
    virtual ~IPathFinder() = default;

    virtual bool FindPath(OSMGraph& graph) = 0;
};

enum class PathfindingModel {
    Dijkstra = 0,
    AStar = 1
};

void PathFinder(OSMGraph& graph, PathfindingModel model);