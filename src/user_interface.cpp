#include "user_interface.h"

#include "raylib.h"
#define RAYGUI_IMPLEMENTATION
#include "../vendor/raygui.h"
#include "Window.h"

constexpr float BORDER_SPACING = 0.05;
constexpr float WINDOW_WIDTH = 0.25;

void DrawUserInterface(const Window &window, Vector2 &dpi) {
    const auto SCREEN_X = static_cast<float>(window.width);
    const auto SCREEN_Y = static_cast<float>(window.height);

    const auto WINDOW_START_X = SCREEN_X * BORDER_SPACING;
    const auto WINDOW_START_Y = SCREEN_Y * BORDER_SPACING;
    const auto WINDOW_LENGTH_X = SCREEN_X * WINDOW_WIDTH;
    const auto WINDOW_LENGTH_Y = SCREEN_Y - (2 * WINDOW_START_Y);

    const Rectangle BOX = {.x=WINDOW_START_X, .y=WINDOW_START_Y, .width=WINDOW_LENGTH_X, .height=WINDOW_LENGTH_Y};



    GuiDrawRectangle(BOX, 3, BLACK, GRAY);
}