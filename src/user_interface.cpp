#include "user_interface.h"

#include "raylib.h"
#define RAYGUI_IMPLEMENTATION
#include <algorithm>
#include <format>

#include "raymath.h"
#include "../vendor/raygui.h"
#include "Window.h"

constexpr float BORDER_SPACING = 0.01;
constexpr float WINDOW_WIDTH = 0.20;

PathfindingModel modelSelection = PathfindingModel::Dijkstra;
bool modelDropdownEdit = false;

void DrawCursor(const Rectangle& userInterface, const Vector2 &mouseWorldPos, Vector2 mousePos);
void DrawRouteInfo(const Rectangle& userInterface, const Vector2 &mousePos);
void DrawDebugInfo(const Rectangle& userInterface, const Window& window);

void DrawUserInterface(const Window &WINDOW, const Vector2 &mouseWorldPos) {
    // We need to set a minimum size for UI elements which will be done "globally" here
    constexpr std::pair<float, float> MIN_UI_SIZE = {200.F, 600.F};

    const auto SCREEN_X = static_cast<float>(WINDOW.width);
    const auto SCREEN_Y = static_cast<float>(WINDOW.height);

    // Only fetch mouse position once for UI logic, might save some compute.
    const auto MOUSE_POS = GetMousePosition();

    const auto WINDOW_START_X = SCREEN_X * BORDER_SPACING;
    const auto WINDOW_START_Y = SCREEN_Y * BORDER_SPACING;
    const auto windowLengthX = std::max(SCREEN_X * WINDOW_WIDTH, MIN_UI_SIZE.first);
    const auto windowLengthY = std::max(SCREEN_Y - (2 * WINDOW_START_Y), MIN_UI_SIZE.second);

    const Rectangle BOX = {.x=WINDOW_START_X, .y=WINDOW_START_Y, .width=windowLengthX, .height=windowLengthY};

    GuiDrawRectangle(BOX, 3, GRAY, RAYWHITE);

    // These elements make up the UI and must not overlap as that will cause collisions.
    DrawCursor(BOX, mouseWorldPos, MOUSE_POS);
    DrawDebugInfo(BOX, WINDOW);
    DrawRouteInfo(BOX, MOUSE_POS);
}

// Only draw the cursor if we are not hovering over UI elements.
void DrawCursor(const Rectangle& userInterface, const Vector2 &mouseWorldPos, const Vector2 mousePos) {

    const Rectangle mRect = {.x=mousePos.x - 1, .y=mousePos.y - 1, .width=2, .height=2};

    if (CheckCollisionRecs(userInterface, mRect)) {
        return;
    }

    DrawCircleV(mousePos, 4, DARKGRAY);

    DrawTextEx(
        GetFontDefault(),
        TextFormat("[%.2f, %.2f]", mouseWorldPos.x, mouseWorldPos.y),
        Vector2Add(GetMousePosition(), (Vector2){.x=-44, .y=-24}), 20, 2, BLACK
    );
}

void DrawDebugInfo(const Rectangle& userInterface, const Window& window) {
    if (!window.showDebug) {
        return;
    }

    const Vector2 START_V = {
        .x=userInterface.x + (userInterface.width * 0.05F),
        .y=userInterface.y + (userInterface.height * 0.825F)};

    const Rectangle DEBUG_BOX = {
        .x=START_V.x ,
        .y=START_V.y ,
        .width=userInterface.width * 0.9F,
        .height=userInterface.height * 0.15F
    };

    const auto currentFPS = GetFPS();


    GuiDrawRectangle(
        DEBUG_BOX,
        2,
        {150, 50, 50, 255 },
        {245, 181, 39, 230}
        );

    // Below is the actual debug information, Constant spacing seems better for this so the ofsets are in pixels.

    DrawText(
        "DEBUG INFO",
        static_cast<int>(DEBUG_BOX.x) + 10,
        static_cast<int>(DEBUG_BOX.y) + 10,
        20,
        BLACK
    );

    DrawText(
        std::format("FPS: {}", currentFPS).c_str(),
        static_cast<int>(DEBUG_BOX.x) + 10,
        static_cast<int>(DEBUG_BOX.y) + 40,
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

void DrawRouteInfo(const Rectangle& userInterface, const Vector2 &mousePos) {



    // GuiTextBox();

    if (GuiDropdownBox(
        (Rectangle){
            userInterface.x + 10, 
            userInterface.y + 10, 
            userInterface.width - 20, 
            40
        }, 
        "Dijkstra;A Star", 
        reinterpret_cast<int*>(&modelSelection), 
        modelDropdownEdit))
    { 
        modelDropdownEdit = !modelDropdownEdit;
    }

}