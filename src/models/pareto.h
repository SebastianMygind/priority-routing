#pragma once
#include "../osm/graph.h"
#include "../path_finder.h"

class Pareto : public IPathFinder
{
public:
    bool FindPath(OSMGraph& graph, UserInterface& ui) override;
};
