#include "path_finder.h"
#include "models/dijkstra.h"
#include "models/a_star.h"
#include "models/weighted.h"
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
        default:
            spdlog::error("Invalid pathfinding model selected");
            return;
    }

    pathfindingThread = std::thread([&graph, &ui, pathfinder = std::move(pathfinder)]() mutable
    {
        ui.ActivateLoader();
        threadDone.store(false);
        const auto timeStart = std::chrono::high_resolution_clock::now();
        pathfinder->FindPath(graph, ui);
        const auto timeEnd = std::chrono::high_resolution_clock::now();
        ui.SetDebugModelTime(timeEnd - timeStart);
        threadDone.store(true);
        ui.DeactivateLoader();
    });

    pathfindingThread.detach();
};