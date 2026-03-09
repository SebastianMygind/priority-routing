#pragma once
#include "../osm/graph.h"
#include "../path_finder.h"

class Dijkstra : public IPathFinder
{
public:
    bool FindPath(
        OSMGraph& graph,
        uint64_t start_node,
        uint64_t end_node,
        std::set<uint64_t>& out_path) override;
};
