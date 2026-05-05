#pragma once

#include <cstring>
#include <chrono>
#include <functional>
#include "timer.h"

#include "raylib.h"


#include "Window.h"
#include "path_finder.h"
#include "../vendor/raygui.h"

using ms_duration = std::chrono::duration<double, std::milli>;

class UserInterface 
{
public:
    UserInterface() = default;
    ~UserInterface();

    // Fixes a clang-tidy warning, taken from Gemini.

    // 1. Delete Copy Constructor (Prevents accidental duplication)
    UserInterface(const UserInterface&) = delete;

    // 2. Delete Copy Assignment (Prevents UI = otherUI)
    UserInterface& operator=(const UserInterface&) = delete;

    // 3. Default Move Constructor (Allows transferring the UI)
    UserInterface(UserInterface&&) noexcept = default;

    // 4. Default Move Assignment
    UserInterface& operator=(UserInterface&&) noexcept = default;

    void DrawUserInterface(const Window &window);
    void SetupUI(const char* file);
    void UpdateLockState();
    void ActivateLoader();
    void DeactivateLoader();

    [[nodiscard]] bool KeyboardInUI() const { return originTextboxEdit || destinationTextboxEdit; }
    [[nodiscard]] static bool MouseInUI()    { return !GuiIsLocked(); }
    bool IsUpdated();

    // Textbox getters/setters

    void SetOrigin(const std::string& text)      { strncpy(originTextboxText, text.c_str(), textboxSize);      }
    void SetDestination(const std::string& text) { strncpy(destinationTextboxText, text.c_str(), textboxSize); }
    [[nodiscard]] std::string GetOrigin()         const { return originTextboxText;      }
    [[nodiscard]] std::string GetDestination()    const { return destinationTextboxText; }

    // Model getters

    [[nodiscard]] PathfindingModel GetModel()    const { return (PathfindingModel)modelSelectionIndex; }
    ObjectiveList GetObjectives();

    [[nodiscard]] auto GetPathIndex() const { return pathSelectionIndex; }
    auto SetPathCount(auto count) { pathCount = count; }

    // Debug setters

    auto SetDebugModelTime(auto duration) { debugModelTime = duration; }
    auto SetDebugTotalNodes(auto nodes)   { debugTotalNodes = nodes;   }
    auto SetDebugTotalWays(auto ways)     { debugTotalWays = ways;     }
    auto SetDebugRenderNodes(auto nodes)  { debugRenderNodes = nodes;  }
    auto SetDebugRenderedWays(auto ways)  { debugRenderedWays = ways;  }
    auto SetDebugMouseCoords(auto lat, auto lon)  { debugMouseWorldPos = {lat, lon}; }
    auto SetPOIText(const std::string& text)     { poiText = text; }

    void SetPathSelectionCallback(std::function<void(int)> callback) { pathSelectionCallback = std::move(callback); }

    // Visibility toggles

    void ToggleUI()    { showUI    = !showUI;    }
    void ToggleDebug() { showDebug = !showDebug; }

private:

    Rectangle uiRect{};
    Vector2 mousePos{};
    Font fontText{}, fontHeading{};

    // State for showing the loading spinner when pathfinding
    Image spinnerImage{};
    Texture2D spinnerTexture{};
    Timer spinnerFrameTimer{8};
    int currentFrame = 0;
    int spinnerFrameCount = 0;
    bool pathfindingInProgress = false;
    std::chrono::steady_clock::time_point lastCompletion = std::chrono::steady_clock::now();
    // in nanoseconds
    static constexpr std::chrono::nanoseconds showTime{1'000'000'000};

    Texture2D checkTexture{};

    // Should match the PathfindingModel enum in path_finder.h
    int modelSelectionIndex = 2;
    int pathSelectionIndex = 0;
    int pathCount = 0;

    ObjectiveList objWeightedSum;
    ObjectiveList objPareto;

    ms_duration debugModelTime{0};
    size_t debugTotalNodes = 0;
    size_t debugTotalWays = 0;
    size_t debugRenderNodes = 0;
    size_t debugRenderedWays = 0;
    Vector2 debugMouseWorldPos{.x=0, .y=0};
    std::string poiText;

    bool showDebug = true;
    bool showUI    = true;
    
    bool modelSelectionEdit = false;
    bool originTextboxEdit = false;
    bool destinationTextboxEdit = false;
    bool wasPreviouslyEditing = false;
    bool showNewObjective = false;

    std::function<void(int)> pathSelectionCallback;

    static constexpr int textboxSize = 128;
    char originTextboxText[textboxSize] = "";
    char destinationTextboxText[textboxSize] = "";

    float elementX{};
    float elementY(int type);
    float elementWidth{};
    float accumulator{};

    void DrawRouteInfo();
    void DrawDebugInfo();
    void DrawPathLoader(const Window &window);

    void DrawCustomHeading(const char* text);
    void DrawCustomText(const char* text);
    void DrawCustomTextbox(char* text, bool& edit);
    void DrawCustomSelection(const char* text, float posY, int* selection, bool& edit);
    void DrawCustomSlider(float* value);
    void DrawCustomPather(int& index, int count);
    void DrawObjecive(ObjectiveList& objectives, int index, bool removable);
    void DrawObjeciveWeight(ObjectiveList& objectives, int index, bool removable);
};