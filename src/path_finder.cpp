#include "path_finder.h"
#include "models/dijkstra.h"
#include "models/a_star.h"
#include "models/weighted.h"
#include "spdlog/spdlog.h"
#include <chrono>
#include <memory>
#include <thread>

void PathFinder(OSMGraph& graph, UserInterface& ui)
{
    std::unique_ptr<IPathFinder> pathfinder = nullptr;
    PathfindingModel model = static_cast<PathfindingModel>(ui.GetModel());

    switch (model)
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

    std::thread ([&graph, &ui, pathfinder = std::move(pathfinder)]() mutable
    {
        auto time_start = std::chrono::high_resolution_clock::now();
        pathfinder->FindPath(graph, ui);
        auto time_end = std::chrono::high_resolution_clock::now();
        ui.SetDebugModelTime(time_end - time_start);
    }).detach();
};