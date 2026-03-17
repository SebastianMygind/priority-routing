#pragma once
#include "osm/graph.h"
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

double Haversine(const OSMNode& a, const OSMNode& b);
std::optional<double> ParseMaxSpeed(const std::string& value);
double EuclideanDistance(const OSMNode& a, const OSMNode& b);
void PathFinder(OSMGraph& graph, PathfindingModel model);