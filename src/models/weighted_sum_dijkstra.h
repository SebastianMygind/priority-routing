#pragma once

#include "../osm/graph.h"
#include "../path_finder.h"

class WeightedDijkstra : public IPathFinder
{
public:
    bool FindPath(OSMGraph& graph) override;
};


