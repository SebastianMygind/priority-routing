#pragma once
#include "../Graph.h"
#include "../path_finder.h"

class Dijkstra : public IPathFinder
{
public:
    bool FindPath(
        Graph& graph,
        uint64_t start_node,
        uint64_t end_node,
        std::set<uint64_t>& out_path) override;
};
