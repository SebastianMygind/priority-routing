#pragma once
#include "osm/graph.h"
#include "user_interface.h"
#include <sstream>
#include <optional>

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

void PathFinder(OSMGraph& graph, UserInterface& ui);