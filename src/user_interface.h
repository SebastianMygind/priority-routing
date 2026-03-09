#pragma once
#include "Graph.h"
#include "raylib.h"
#include "Window.h"
#include "path_finder.h"

struct UIState {
    PathfindingModel modelSelection = PathfindingModel::Dijkstra;
    bool modelDropdownEdit = false;
};

void DrawUserInterface(const Window &window, const Vector2 &mWorldPos, const Graph& graph, UIState &state);