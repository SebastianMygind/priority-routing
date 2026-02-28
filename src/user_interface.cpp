#include "user_interface.h"

#include "raylib.h"
#define RAYGUI_IMPLEMENTATION
#include "raymath.h"
#include "../vendor/raygui.h"
#include "Window.h"

constexpr float BORDER_SPACING = 0.01;
constexpr float WINDOW_WIDTH = 0.20;

void DrawCursor(const Rectangle& userInterface, const Vector2 &mouseWorldPos);

void DrawUserInterface(const Window &window, const Vector2 &mouseWorldPos) {
    const auto SCREEN_X = static_cast<float>(window.width);
    const auto SCREEN_Y = static_cast<float>(window.height);

    const auto WINDOW_START_X = SCREEN_X * BORDER_SPACING;
    const auto WINDOW_START_Y = SCREEN_Y * BORDER_SPACING;
    const auto WINDOW_LENGTH_X = SCREEN_X * WINDOW_WIDTH;
    const auto WINDOW_LENGTH_Y = SCREEN_Y - (2 * WINDOW_START_Y);

    const Rectangle BOX = {.x=WINDOW_START_X, .y=WINDOW_START_Y, .width=WINDOW_LENGTH_X, .height=WINDOW_LENGTH_Y};

    GuiDrawRectangle(BOX, 3, GRAY, RAYWHITE);
    DrawCursor(BOX, mouseWorldPos);
}

// Only draw the cursor if we are not hovering over UI elements.
void DrawCursor(const Rectangle& userInterface, const Vector2 &mouseWorldPos) {

    const auto mPos = GetMousePosition();

    const Rectangle mRect = {.x=mPos.x - 1, .y=mPos.y - 1, .width=2, .height=2};

    if (CheckCollisionRecs(userInterface, mRect)) {
        return;
    }

    DrawCircleV(mPos, 4, DARKGRAY);

    DrawTextEx(
        GetFontDefault(),
        TextFormat("[%.2f, %.2f]", mouseWorldPos.x, mouseWorldPos.y),
        Vector2Add(GetMousePosition(), (Vector2){.x=-44, .y=-24}), 20, 2, BLACK
    );
}