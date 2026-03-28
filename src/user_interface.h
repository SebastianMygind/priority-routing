#pragma once
#include "osm/graph.h"
#include "osm/renderer.h"
#include "raylib.h"
#include "Window.h"
#include "path_finder.h"

// Height of elements, effectively their size

constexpr int HEADING_HEIGHT = 30;
constexpr int TEXT_HEIGHT    = 20;
constexpr int BOX_HEIGHT     = 40;
constexpr int SLIDER_HEIGHT  = 20;

// Padding between elements, horizontal and vertical

constexpr std::pair H_PAD = {10, -20};
constexpr int       V_PAD = 5;

// Used to scale the UI accordingly

constexpr std::pair UI_MIN_SIZE   = {200.F, 600.F};
constexpr float     UI_MULTIPLIER = 0.20;
constexpr float     UI_DEBUG_SIZE = 210.F;

// Accumulator types are used with elementY to determine the y position of the next element
// Use the type of the element you're drawing or call elementY(padding) to add spacing between elements

enum AccumulatorTypes 
{
    heading = HEADING_HEIGHT + V_PAD, 
    text = TEXT_HEIGHT + V_PAD,
    box = BOX_HEIGHT + V_PAD,
    slider = SLIDER_HEIGHT + V_PAD,
    padding = 4 * V_PAD
};


class UserInterface 
{
public:

    void DrawUserInterface(const Window &window, const Vector2 &mouseWorldPos, const OSMGraph &graph,
                           const OSMRenderer &renderer);

    auto MouseInUI()    const { return CheckCollisionPointRec(mousePos, uiRect) && showUI; }
    auto KeyboardInUI() const { return textboxEdit; }

    auto GetModel()    const { return modelSelection; }
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

    Rectangle uiRect;
    Vector2 mousePos;

    float screenX;
    float screenY;

    float objDistance = 0.5;
    float objTime     = 0.5;
    float objScenery  = 0.5;
    float objTourism  = 0.5;
    float objComfort  = 0.5;

    bool showDebug = false;
    bool showQuad  = false;
    bool showUI    = true;
    
    bool modelDropdownEdit = false;
    bool textboxEdit = false;
    bool sliderEdit  = false;

    void DrawRouteInfo();
    void DrawCursor(const Vector2 &mouseWorldPos);
    void DrawDebugInfo(const OSMGraph &graph, const OSMRenderer &renderer);
};