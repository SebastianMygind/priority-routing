#pragma once

#include "raylib.h"

// Setup function, must be called after the raylib window has been initialized.
void SetupFontConfig();

// This functions just wraps Raylibs DrawTextEx with a custom chosen Font
void DrawCustomText(const char *text, int posX, int posY, int fontSize, Color color);
