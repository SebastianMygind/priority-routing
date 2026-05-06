#include "path_finder.h"
#include "models/dijkstra.h"
#include "models/pareto.h"
#include "models/a_star.h"
#include "models/weighted.h"
#include "user_interface.h"
#include "spdlog/spdlog.h"
#include <chrono>
#include <memory>

std::thread pathfindingThread;
std::atomic<bool> threadDone = true;
std::atomic<bool> threadKill = false;

void PathFinder(OSMGraph& graph, UserInterface& ui)
{
    // Check if thread is already running, if so, kill it
    if (!threadDone.load())
    {
        threadKill.store(true);
        while (!threadDone.load()) {}
        threadKill.store(false);
    }

    std::unique_ptr<IPathFinder> pathfinder = nullptr;

    switch (static_cast<PathfindingModel>(ui.GetModel()))
    {
        case PathfindingModel::Dijkstra:
            pathfinder = std::make_unique<Dijkstra>();
            break;
        case PathfindingModel::AStar:
            pathfinder = std::make_unique<AStar>();
            break;
        case PathfindingModel::Weighted:
            pathfinder = std::make_unique<Weighted>();
            break;
        case PathfindingModel::Pareto:
            pathfinder = std::make_unique<Pareto>();
            break;
        default:
            spdlog::error("Invalid pathfinding model selected");
            return;
    }

    pathfindingThread = std::thread([&graph, &ui, pathfinder = std::move(pathfinder)]() mutable
    {
        ui.ActivateLoader();
        threadDone.store(false);
        const auto timeStart = std::chrono::high_resolution_clock::now();
        graph.ClearPath();
        pathfinder->FindPath(graph, ui.GetObjectives());
        const auto timeEnd = std::chrono::high_resolution_clock::now();
        ui.SetDebugModelTime(timeEnd - timeStart);
        threadDone.store(true);
        ui.DeactivateLoader();
        ui.SetPathCount(graph.GetPathCount());
    });

    pathfindingThread.detach();
};

double DistanceObjective(OSMGraph& graph, const OSMNode& a, const OSMNode& b, const OSMWay& way, double distance)
{
    return Normalize(distance, 0, 750);;
}

double TravelTimeObjective(OSMGraph& graph, const OSMNode& a, const OSMNode& b, const OSMWay& way, double distance)
{
    auto speedTag = way.tags.find("maxspeed");
    auto highwayTag = way.tags.find("highway");
    double speed = (speedTag != way.tags.end())
        ? ParseMaxSpeed(speedTag->second)
        : GetDefaultSpeed(highwayTag->second);
    auto time = distance / speed;
    return Normalize(time, 0, 150);
}

double TrafficSignalObjective(OSMGraph& graph, const OSMNode& a, const OSMNode& b, const OSMWay& way, double distance)
{
    auto highwayTagB = b.tags.find("highway");
    auto lights = (highwayTagB != b.tags.end() && highwayTagB->second == "traffic_signals") ? 1.0 : 0.0;
    spdlog::info("{}", lights);
    return Normalize(lights, 0, 2);
}

double LitRoadObjective(OSMGraph& graph, const OSMNode& a, const OSMNode& b, const OSMWay& way, double distance)
{
    auto litTag = way.tags.find("lit");
    auto lit = ((litTag != way.tags.end() && litTag->second == "yes") ? 1.0 : 10.0) * distance;
    return Normalize(lit, 0, 1500);
}

double RoadSmoothnessObjective(OSMGraph& graph, const OSMNode& a, const OSMNode& b, const OSMWay& way, double distance)
{
    auto smoothnessTag = way.tags.find("smoothness");
    auto smoothness = (smoothnessTag != way.tags.end() ? GetRoadSmoothness(smoothnessTag->second) : 5.0) * distance;
    return Normalize(smoothness, 0, 3000);
}

double GasStationObjective(OSMGraph& graph, const OSMNode& a, const OSMNode& b, const OSMWay& way, double distance)
{
    std::pair<OSMNodeID, double> nearestFuel = graph.GetNearestNode("amenity=fuel", b.location);
    return Normalize(nearestFuel.second, 100, 500);
}

double CafeObjective(OSMGraph& graph, const OSMNode& a, const OSMNode& b, const OSMWay& way, double distance)
{
    std::pair<OSMNodeID, double> nearestCafe = graph.GetNearestNode("amenity=cafe", b.location);
    return Normalize(nearestCafe.second, 100, 500);
}

double TourismObjective(OSMGraph& graph, const OSMNode& a, const OSMNode& b, const OSMWay& way, double distance)
{
    std::pair<OSMNodeID, double> nearestTourism = graph.GetNearestNode("tourism", b.location);
    return Normalize(nearestTourism.second, 100, 500);
}