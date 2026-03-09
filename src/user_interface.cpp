#include "user_interface.h"

#include "raylib.h"
#define RAYGUI_IMPLEMENTATION
#include <algorithm>
#include <format>

#include "Graph.h"
#include "raymath.h"
#include "../vendor/raygui.h"
#include "Window.h"

constexpr float BORDER_SPACING = 0.01;
constexpr float WINDOW_WIDTH = 0.20;
// We need to set a minimum size for UI elements which will be done "globally" here
constexpr std::pair<float, float> MIN_UI_SIZE = {200.F, 600.F};


void DrawCursor(const Rectangle& userInterface, const Vector2 &mouseWorldPos, Vector2 mousePos);
void DrawRouteInfo(const Rectangle& userInterface, const Vector2 &mousePos, const Graph& graph, UIState& state);
void DrawDebugInfo(const Rectangle& userInterface, const Window& window);

void DrawUserInterface(const Window &window, const Vector2 &mWorldPos, const Graph& graph, UIState& state) {

    const auto screenX = static_cast<float>(window.width);
    const auto screenY = static_cast<float>(window.height);

    // Only fetch mouse position once for UI logic, might save some compute.
    const auto mPos = GetMousePosition();

    const auto winStartX = screenX * BORDER_SPACING;
    const auto winStartY = screenY * BORDER_SPACING;
    const auto winLenX = std::max(screenX * WINDOW_WIDTH, MIN_UI_SIZE.first);
    const auto winLenY = std::max(screenY - (2 * winStartY), MIN_UI_SIZE.second);

    const Rectangle uiRect = {.x = winStartX, .y = winStartY, .width = winLenX, .height = winLenY};

    GuiDrawRectangle(uiRect, 3, GRAY, RAYWHITE);

    // These elements make up the UI and must not overlap as that will cause collisions.
    DrawCursor(uiRect, mWorldPos, mPos);
    DrawDebugInfo(uiRect, window);
    DrawRouteInfo(uiRect, mPos, graph, state);
}

// Only draw the cursor if we are not hovering over UI elements.
void DrawCursor(const Rectangle &uiRect, const Vector2 &mWorldPos, const Vector2 mPos) {
    const Rectangle mRect = {.x = mPos.x - 1, .y = mPos.y - 1, .width = 2, .height = 2};

    if (CheckCollisionRecs(uiRect, mRect)) {
        return;
    }

    DrawCircleV(mPos, 4, DARKGRAY);

    DrawTextEx(
        GetFontDefault(),
        TextFormat("[%.2f, %.2f]", mWorldPos.x, mWorldPos.y),
        Vector2Add(GetMousePosition(), {.x = -44, .y = -24}), 20, 2, BLACK
    );
}

void DrawDebugInfo(const Rectangle &uiRect, const Window &window) {
    if (!window.showDebug) {
        return;
    }

    const Vector2 startVec = {
        .x = uiRect.x + (uiRect.width * 0.05F),
        .y = uiRect.y + (uiRect.height * 0.825F)
    };

    const Rectangle debugRect = {
        .x = startVec.x,
        .y = startVec.y,
        .width = uiRect.width * 0.9F,
        .height = uiRect.height * 0.15F
    };

    const auto currentFPS = GetFPS();


    GuiDrawRectangle(
        debugRect,
        2,
        {.r=150, .g=50, .b=50, .a=255},
        {.r=245, .g=181, .b=39, .a=230}
    );

    // Below is the actual debug information, Constant spacing seems better for this so the offsets are in pixels.

    DrawText(
        "DEBUG INFO",
        static_cast<int>(debugRect.x) + 10,
        static_cast<int>(debugRect.y) + 10,
        20,
        BLACK
    );

    DrawText(
        std::format("FPS: {}", currentFPS).c_str(),
        static_cast<int>(debugRect.x) + 10,
        static_cast<int>(debugRect.y) + 40,
        18,
        BLACK
    );
}

// todo! add back positional ui info
// DrawTextEx(
//  GetFontDefault(),
//  TextFormat("Selected Node: \n A:%i, B:%i", graph.selected_node_a, graph.selected_node_b),
//  {10, 10}, 20, 2, BLACK
//  );

void DrawRouteInfo(const Rectangle& uiRect, const Vector2 &mPos, const Graph &graph, UIState &state) {



    // GuiTextBox();

    char* text = "Hello World";
    const Rectangle boxBounds = {
        .x = uiRect.x + 10,
        .y = uiRect.y + 10,
        .width = uiRect.width * 0.8F,
        .height = 30.F
    };

    GuiTextBox(boxBounds, text, 16, false);

    // Model selection dropdown

    int modelSelectionIndex = static_cast<int>(state.modelSelection);
    if (GuiDropdownBox(
        (Rectangle){
            .x=uiRect.x + 10,
            .y=uiRect.y + 10,
            .width=uiRect.width - 20,
            .height=40
        },
        "Dijkstra;A Star",
        &modelSelectionIndex,
        state.modelDropdownEdit))
    {
        state.modelDropdownEdit = !state.modelDropdownEdit;
        state.modelSelection = static_cast<PathfindingModel>(modelSelectionIndex);
    }
}