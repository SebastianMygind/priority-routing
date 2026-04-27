#pragma once
#include "raylib.h"
#include "../vendor/raygui.h"
#include "Window.h"
#include <cstring>
#include <chrono>

#include "timer.h"

using ms_duration = std::chrono::duration<double, std::milli>;

class UserInterface 
{
public:

    ~UserInterface();

    void DrawUserInterface(const Window &window);
    void SetupUI(const char* file);
    void UpdateLockState();
    void ActivateLoader();
    void DeactivateLoader();

    bool KeyboardInUI() const { return originTextboxEdit || destinationTextboxEdit; }
    bool MouseInUI()    const { return !GuiIsLocked(); }
    bool IsUpdated();

    // Textbox getters/setters

    void SetOrigin(std::string text)      { strncpy(originTextboxText, text.c_str(), textboxSize);      }
    void SetDestination(std::string text) { strncpy(destinationTextboxText, text.c_str(), textboxSize); }
    std::string GetOrigin()         const { return std::string(originTextboxText);      }
    std::string GetDestination()    const { return std::string(destinationTextboxText); }

    // Model getters


    auto GetModel()    const { return modelSelectionIndex; }
    auto GetDistance() const { return objDistance / GetSum();    }
    auto GetTime()     const { return objTime / GetSum();        }
    auto GetLitRoads()  const { return objLitRoads / GetSum();     }
    auto GetSmoothness()  const { return objSmoothness / GetSum();     }
    auto GetGasStation()  const { return objGasStation / GetSum();     }
    auto GetCafe()  const { return objCafe / GetSum();     }
    auto GetTourism()  const { return objTourism / GetSum();     }
    float GetSum()     const { return objDistance + objTime + objLitRoads + objSmoothness + objGasStation + objCafe + objTourism; }

    // Debug setters

    auto SetDebugModelTime(auto duration) { debugModelTime = duration; }
    auto SetDebugTotalNodes(auto nodes)   { debugTotalNodes = nodes;   }
    auto SetDebugTotalWays(auto ways)     { debugTotalWays = ways;     }
    auto SetDebugRenderNodes(auto nodes)  { debugRenderNodes = nodes;  }
    auto SetDebugRenderedWays(auto ways)  { debugRenderedWays = ways;  }
    auto SetDebugMouseCoords(auto lat, auto lon)  { debugMouseWorldPos = {lat, lon}; }
    auto SetPOIText(std::string text)     { poiText = text; }

    // Visibility toggles

    void ToggleUI()    { showUI    = !showUI;    }
    void ToggleDebug() { showDebug = !showDebug; }

private:

    Rectangle uiRect;
    Vector2 mousePos;
    Font fontText, fontHeading;

    // State for showing the loading spinner when pathfinding
    Image spinnerImage;
    Texture2D spinnerTexture;
    Timer spinnerFrameTimer{8};
    int currentFrame = 0;
    int spinnerFrameCount = 0;
    bool pathfindingInProgress = false;
    std::chrono::steady_clock::time_point lastCompletion = std::chrono::steady_clock::now();
    // in nanoseconds
    static constexpr std::chrono::nanoseconds showTime{1'000'000'000};

    Texture2D checkTexture;

    // Should match the PathfindingModel enum in path_finder.h
    int modelSelectionIndex = 2;

    float objDistance = 0.5;
    float objTime     = 0.5;
    float objLitRoads  = 0.5;
    float objSmoothness  = 0.5;
    float objGasStation  = 0.5;
    float objCafe  = 0.5;
    float objTourism  = 0.5;

    ms_duration debugModelTime{0};
    size_t debugTotalNodes = 0;
    size_t debugTotalWays = 0;
    size_t debugRenderNodes = 0;
    size_t debugRenderedWays = 0;
    Vector2 debugMouseWorldPos{0, 0};
    std::string poiText = "";

    bool showDebug = true;
    bool showUI    = true;
    
    bool modelSelectionEdit = false;
    bool originTextboxEdit = false;
    bool destinationTextboxEdit = false;
    bool wasPreviouslyEditing = false;

    static const int textboxSize = 128;
    char originTextboxText[textboxSize] = "";
    char destinationTextboxText[textboxSize] = "";

    float elementX;
    float elementY(int type);
    float elementWidth;
    float accumulator;

    void DrawRouteInfo();
    void DrawDebugInfo();
    void DrawPathLoader(const Window &window);

    void DrawCustomHeading(const char* text);
    void DrawCustomText(const char* text);
    void DrawCustomTextbox(char* text, bool& edit);
    void DrawCustomSelection(const char* text, float posY, int* selection, bool& edit);
    void DrawCustomSlider(float* value);
};