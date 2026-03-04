#include "user_interface.h"

#include "raylib.h"
#define RAYGUI_IMPLEMENTATION
#include <format>

#include "raymath.h"
#include "../vendor/raygui.h"
#include "Window.h"

constexpr float BORDER_SPACING = 0.01;
constexpr float WINDOW_WIDTH = 0.20;

void DrawCursor(const Rectangle& userInterface, const Vector2 &mouseWorldPos, Vector2 mousePos);
void DrawRouteInfo(const Rectangle& userInterface, const Vector2 &mousePos);
void DrawDebugInfo(const Rectangle& userInterface, const Window& window);

void DrawUserInterface(const Window &WINDOW, const Vector2 &mouseWorldPos) {
    // We need to set a minimum size for UI elements which will be done "globally" here
    constexpr std::pair<float, float> MIN_UI_SIZE = {};

    const auto SCREEN_X = static_cast<float>(WINDOW.width);
    const auto SCREEN_Y = static_cast<float>(WINDOW.height);

    // Only fetch mouse position once for UI logic, might save some compute.
    const auto MOUSE_POS = GetMousePosition();

    const auto WINDOW_START_X = SCREEN_X * BORDER_SPACING;
    const auto WINDOW_START_Y = SCREEN_Y * BORDER_SPACING;
    const auto WINDOW_LENGTH_X = SCREEN_X * WINDOW_WIDTH;
    const auto WINDOW_LENGTH_Y = SCREEN_Y - (2 * WINDOW_START_Y);

    const Rectangle BOX = {.x=WINDOW_START_X, .y=WINDOW_START_Y, .width=WINDOW_LENGTH_X, .height=WINDOW_LENGTH_Y};

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

    DrawText(
        std::format("FPS: {}", currentFPS).c_str(),
        200,
        200,
        18,
        BLACK
    );


}

void DrawRouteInfo(const Rectangle& userInterface, const Vector2 &mousePos) {



    // GuiTextBox();
}