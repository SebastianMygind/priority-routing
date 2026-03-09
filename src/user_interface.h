#pragma once
#include "osm/graph.h"
#include "raylib.h"
#include "Window.h"
#include "path_finder.h"

struct UIState {
    PathfindingModel modelSelection = PathfindingModel::Dijkstra;
    bool modelDropdownEdit = false;
};

void DrawUserInterface(const Window &window, const Vector2 &mWorldPos, const OSMGraph& graph, UIState &state);