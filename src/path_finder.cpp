#include "path_finder.h"
#include "models/dijkstra.h"
#include "models/a_star.h"
#include "spdlog/spdlog.h"
#include <chrono>
#include <memory>

void PathFinder(OSMGraph& graph, PathfindingModel model)
{
    std::unique_ptr<IPathFinder> pathfinder = nullptr;

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

    std::chrono::duration<double, std::milli> duration = time_end - time_start;
    spdlog::info("Time taken for pathfinding: {} ms", duration.count());
    
};