#pragma once
#include "../osm/graph.h"
#include "../path_finder.h"

class Dijkstra : public IPathFinder
{
public:
    bool FindPath(OSMGraph& graph, UserInterface& ui) override;
};
