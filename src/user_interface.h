#pragma once
#include "osm/graph.h"
#include "osm/renderer.h"
#include "raylib.h"
#include "Window.h"
#include "path_finder.h"

struct UIState {
    PathfindingModel modelSelection = PathfindingModel::AStar;
    bool modelDropdownEdit = false;
};

void DrawUserInterface(const Window &window, const Vector2 &mWorldPos, const OSMGraph &graph,
                       const OSMRenderer &renderer, UIState &state, bool &globalKeyIsLocked);
