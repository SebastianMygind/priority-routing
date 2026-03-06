#pragma once
#include "raylib.h"
#include "Window.h"
#include "path_finder.h"


extern PathfindingModel modelSelection;
void DrawUserInterface(const Window &WINDOW, const Vector2 &mouseWorldPos);