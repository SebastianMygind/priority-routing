#pragma once
#include "../path_finder.h"

class AStar : public IPathFinder
{
public:
    bool FindPath(OSMGraph& graph, ObjectiveList objectives) override;
};
