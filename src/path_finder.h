#pragma once
#include "osm/graph.h"

#include <functional>
#include <atomic>

using ObjectiveFunc = std::function<double(OSMGraph&, const OSMNode&, const OSMNode&, const OSMWay&, double)>;

struct Objective
{
    std::string   name;
    ObjectiveFunc func;

    float         weight;
};

using ObjectiveList = std::vector<Objective>;


class IPathFinder
{
public:
    virtual ~IPathFinder() = default;
    virtual bool FindPath(OSMGraph& graph, ObjectiveList objectives) = 0;

    void Stop() { running = false; }

protected:
    std::atomic<bool> running = true;
};

enum class PathfindingModel {
    Dijkstra = 0,
    AStar = 1,
    Weighted = 2,
    Pareto = 3,
};

double DistanceObjective(OSMGraph& graph, const OSMNode& a, const OSMNode& b, const OSMWay& way, double distance);
double TravelTimeObjective(OSMGraph& graph, const OSMNode& a, const OSMNode& b, const OSMWay& way, double distance);
double TrafficSignalObjective(OSMGraph& graph, const OSMNode& a, const OSMNode& b, const OSMWay& way, double distance);
double LitRoadObjective(OSMGraph& graph, const OSMNode& a, const OSMNode& b, const OSMWay& way, double distance);
double RoadSmoothnessObjective(OSMGraph& graph, const OSMNode& a, const OSMNode& b, const OSMWay& way, double distance);
double GasStationObjective(OSMGraph& graph, const OSMNode& a, const OSMNode& b, const OSMWay& way, double distance);
double CafeObjective(OSMGraph& graph, const OSMNode& a, const OSMNode& b, const OSMWay& way, double distance);
double TourismObjective(OSMGraph& graph, const OSMNode& a, const OSMNode& b, const OSMWay& way, double distance);