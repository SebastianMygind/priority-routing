#pragma once
#include "osm/graph.h"
#include "osm/renderer.h"
#include "raylib.h"
#include "Window.h"
#include "path_finder.h"

// Height of elements, effectively their size

constexpr int HEADING_HEIGHT = 30;
constexpr int TEXT_HEIGHT = 20;
constexpr int BOX_HEIGHT = 40;
constexpr int SLIDER_HEIGHT = 20;

// Padding between elements, horizontal and vertical

constexpr std::pair<int, int> H_PAD = {10, -20};
constexpr int V_PAD = 5;

class UserInterface 
{
public:

    void DrawUserInterface(const Window &window, const Vector2 &mouseWorldPos, const OSMGraph &graph,
                           const OSMRenderer &renderer, bool &UpdateEditModeLocked);

    bool MouseInUI();
    bool KeyboardInUI() const {return false;};

    auto GetModel() const { return modelSelection; }

    auto GetDistance() const { return objDistance;    }
    auto GetTime()     const { return objTime;        }
    auto GetScenery()  const { return objScenery;     }
    auto GetTourism()  const { return objTourism;     }
    auto GetComfort()  const { return objComfort;     }

    void ToggleDebug() { showDebug = !showDebug; }
    void ToggleQuad()  { showQuad  = !showQuad;  }
    void ToggleUI()    { showUI    = !showUI;    }

    auto GetQuad() const { return showQuad; }

private:

    PathfindingModel modelSelection = PathfindingModel::AStar;

    float objDistance = 0.5;
    float objTime     = 0.5;
    float objScenery  = 0.5;
    float objTourism  = 0.5;
    float objComfort  = 0.5;

    bool showDebug = true;
    bool showQuad  = false;
    bool showUI    = true;
    
    bool modelDropdownEdit = false;
    Rectangle uiRect;
    Vector2 mousePos;

    void DrawCursor(const Vector2 &mouseWorldPos);

    void DrawRouteInfo(bool &globalKeyIsLocked);

    void DrawDebugInfo(const OSMGraph &graph, const OSMRenderer &renderer);
};