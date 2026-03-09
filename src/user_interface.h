#pragma once
#include "Graph.h"
#include "raylib.h"
#include "Window.h"
#include "path_finder.h"

struct UIState {
    PathfindingModel modelSelection = PathfindingModel::Dijkstra;
    bool modelDropdownEdit = false;
};

void DrawUserInterface(const Window &WINDOW, const Vector2 &mouseWorldPos, const Graph& graph, UIState &state);