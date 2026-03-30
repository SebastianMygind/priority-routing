#pragma once
#include "raylib.h"
#include "../vendor/raygui.h"
#include "Window.h"
#include <cstring>
#include <chrono>

typedef std::chrono::duration<double, std::milli> ms_duration;

class UserInterface 
{
public:

    void DrawUserInterface(const Window &window);
    void SetupFontConfig(const char* file);
    void UpdateLockState();

    bool KeyboardInUI() const { return originTextboxEdit || destinationTextboxEdit; }
    bool MouseInUI()    const { return !GuiIsLocked(); }
    bool IsUpdated();

    // Textbox getters/setters

    void SetOrigin(std::string text)      { strncpy(originTextboxText, text.c_str(), sizeof(originTextboxText));           }
    void SetDestination(std::string text) { strncpy(destinationTextboxText, text.c_str(), sizeof(destinationTextboxText)); }
    std::string GetOrigin()         const { return std::string(originTextboxText);      }
    std::string GetDestination()    const { return std::string(destinationTextboxText); }

    // Model getters

    auto GetModel()    const { return modelSelection; }
    auto GetDistance() const { return objDistance;    }
    auto GetTime()     const { return objTime;        }
    auto GetScenery()  const { return objScenery;     }
    auto GetTourism()  const { return objTourism;     }
    auto GetComfort()  const { return objComfort;     }

    // Debug setters

    auto SetDebugModelTime(auto duration) { debugModelTime = duration; }
    auto SetDebugTotalNodes(auto nodes)   { debugTotalNodes = nodes;   }
    auto SetDebugTotalWays(auto ways)     { debugTotalWays = ways;     }
    auto SetDebugRenderNodes(auto nodes)  { debugRenderNodes = nodes; }
    auto SetDebugRenderedWays(auto ways)  { debugRenderedWays = ways;  }
    auto SetDebugMouseCoords(auto lat, auto lon)  { debugMouseWorldPos = {lat, lon}; }

    // Visibility toggles

    void ToggleUI()    { showUI    = !showUI;    }
    void ToggleDebug() { showDebug = !showDebug; }

private:

    Rectangle uiRect;
    Vector2 mousePos;
    Font fontText, fontHeading;
    
    // Should match the PathfindingModel enum in path_finder.h
    int modelSelection = 1;

    float objDistance = 0.5;
    float objTime     = 0.5;
    float objScenery  = 0.5;
    float objTourism  = 0.5;
    float objComfort  = 0.5;

    ms_duration debugModelTime{0};
    size_t debugTotalNodes = 0;
    size_t debugTotalWays = 0;
    size_t debugRenderNodes = 0;
    size_t debugRenderedWays = 0;
    Vector2 debugMouseWorldPos{0, 0};

    bool showDebug = true;
    bool showUI    = true;
    
    bool modelDropdownEdit = false;
    bool originTextboxEdit = false;
    bool destinationTextboxEdit = false;
    bool wasPreviouslyEditing = false;

    char originTextboxText[128] = "";
    char destinationTextboxText[128] = "";

    void DrawRouteInfo();
    void DrawDebugInfo();
};