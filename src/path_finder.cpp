#include "path_finder.h"
#include "models/dijkstra.h"
#include "models/a_star.h"
#include "spdlog/spdlog.h"
#include <chrono>
#include <memory>

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
        default:
            spdlog::error("Invalid pathfinding model selected");
            return;
    }

    auto time_start = std::chrono::high_resolution_clock::now();
    pathfinder->FindPath(graph);
    auto time_end = std::chrono::high_resolution_clock::now();

    ui.SetDebugModelTime(time_end - time_start);    
};