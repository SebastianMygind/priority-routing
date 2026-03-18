#include "text.h"

#include "raylib.h"
#include "raygui.h"

static Font font;

constexpr int FONT_MULTIPLE = 13;

// This function can only be called after InitWindow has been run from main.
void SetupFontConfig() {
    font = LoadFontEx("../fonts/JetBrainsMono-Regular.ttf", FONT_MULTIPLE * 5, nullptr, 0);
    // Using raygui elements with this font looks really weird,
    // I think there are scaling issues with fonts, so not sure if we should change it here or at all
    //GuiSetFont(LoadFontEx("../fonts/JetBrainsMono-Regular.ttf", 13, nullptr, 0));
}

void DrawCustomText(const char *text, const int posX, const int posY, const int fontSize, const Color color) {
    DrawTextEx(
        font,
        text,
        {.x = static_cast<float>(posX), .y = static_cast<float>(posY)},
        static_cast<float>(fontSize),
        1.2,
        color);
}
