#pragma once
#include "osm/graph.h"
#include "osm/renderer.h"
#include "raylib.h"
#include "Window.h"
#include "path_finder.h"

struct UIState {
    PathfindingModel modelSelection = PathfindingModel::AStar;
    bool modelDropdownEdit = false;
    float objDistance = 0.5;
    float objTime = 0.5;
    float objScenery = 0.5;
    float objTourism = 0.5;
    float objComfort = 0.5;
};

void DrawUserInterface(const Window &window, const Vector2 &mWorldPos, const OSMGraph &graph,
                       const OSMRenderer &renderer, UIState &state, bool &globalKeyIsLocked);
