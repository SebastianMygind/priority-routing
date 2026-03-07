#pragma once
#include "raylib.h"
#include "Window.h"
#include "path_finder.h"


struct UIState {
    PathfindingModel modelSelection = PathfindingModel::Dijkstra;
    bool modelDropdownEdit = false;
};


void DrawUserInterface(const Window &WINDOW,
                       const Vector2 &mouseWorldPos,
                       UIState &state);