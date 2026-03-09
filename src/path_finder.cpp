#include "path_finder.h"
#include "models/dijkstra.h"
#include "spdlog/spdlog.h"

void PathFinder(OSMGraph& graph, PathfindingModel model)
{
    IPathFinder* pathfinder = nullptr;

    switch (model)
    {
        case PathfindingModel::Dijkstra:
            pathfinder = new Dijkstra();
            break;
        //case PathfindingModel::AStar:
        //    pathfinder = new AStar();
        //    break;
        default:
            spdlog::error("Invalid pathfinding model selected");
            return;
    }

    pathfinder->FindPath(graph);
};