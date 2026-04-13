#pragma once
#include "../osm/graph.h"
#include "../path_finder.h"

class AStar : public IPathFinder
{
public:
    bool FindPath(OSMGraph& graph, UserInterface& ui) override;
};
