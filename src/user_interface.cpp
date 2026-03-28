#include "user_interface.h"

#include "raylib.h"
#include <algorithm>
#include <format>

#include "raymath.h"
#include "../vendor/raygui.h"
#include "Window.h"

#include "widgets/text_box.h"
#include "widgets/text.h"
#include "spdlog/spdlog.h"

static TextBox textBoxFrom;
static TextBox textBoxTo;

void UserInterface::DrawUserInterface(const Window &window,
                       const Vector2 &mouseWorldPos,
                       const OSMGraph &graph,
                       const OSMRenderer &renderer) 
{
    // Skip drawing if UI is hidden

    if (!showUI) {
        return;
    }

    // Set up the UI box dimensions based on the window size

    screenX = static_cast<float>(window.width);
    screenY = static_cast<float>(window.height);

    const auto winX = H_PAD.first;
    const auto winY = H_PAD.first;
    const auto winWidth  = std::max(screenX * UI_MULTIPLIER, UI_MIN_SIZE.first);
    const auto winHeight = std::max(screenY + H_PAD.second, UI_MIN_SIZE.second);

    // Set up the UI box and mouse pos used throughout the class

    uiRect = {.x = winX, .y = winY, .width = winWidth, .height = winHeight};
    mousePos = GetMousePosition();

    // Draw the UI

    DrawCursor(mouseWorldPos);
    DrawRouteInfo();
    DrawDebugInfo(graph, renderer);
}

void UserInterface::DrawCursor(const Vector2 &mouseWorldPos) 
{
    if (CheckCollisionPointRec(mousePos, uiRect)) {
        return;
    }

    DrawCircleV(mousePos, 4, DARKGRAY);

    DrawTextEx(
        GetFontDefault(), TextFormat("[%.2f, %.2f]", mouseWorldPos.x, mouseWorldPos.y),
        Vector2Add(GetMousePosition(), {.x = -44, .y = -24}), 20, 2, BLACK);
}

void UserInterface::DrawRouteInfo() 
{
    // Define sizes and positions of UI elements

    const auto elementX = uiRect.x + H_PAD.first;
    const auto elementWidth = uiRect.width + H_PAD.second;

    auto accumulator = uiRect.y;
    auto elementY = [&](int type) {
        int current = accumulator;
        accumulator += type;
        return current;
    };

    // The beginning of the ui panel
    
    GuiPanel(uiRect, nullptr);

    // "Location" elements - heading text and two text boxes for origin and destination

    elementY(padding);
    DrawCustomText("Location", elementX, elementY(heading), HEADING_HEIGHT, BLACK);
    DrawRectangle(elementX, elementY(padding), elementWidth, 1, GRAY);

    DrawCustomText("Origin", elementX, elementY(text), TEXT_HEIGHT, BLACK);
    textBoxFrom.Update(Rectangle{.x = elementX,
                                 .y = static_cast<float>(elementY(box)),
                                 .width = elementWidth,
                                 .height = BOX_HEIGHT}, 
                                 mousePos, textboxEdit);

    DrawCustomText("Destination", elementX, elementY(text), TEXT_HEIGHT, BLACK);
    textBoxTo.Update(Rectangle{.x = elementX,
                               .y = static_cast<float>(elementY(box)),
                               .width = elementWidth,
                               .height = BOX_HEIGHT}, 
                               mousePos, textboxEdit);

    // "Algorithm" elements - heading text, dropdown for model selection and sliders for the different weights

    elementY(padding);
    DrawCustomText("Algorithm", elementX, elementY(heading), HEADING_HEIGHT, BLACK);
    DrawRectangle(elementX, elementY(padding), elementWidth, 1, GRAY);

    DrawCustomText("Model", elementX, elementY(text), TEXT_HEIGHT, BLACK);
    auto modelSelectionY = elementY(box); // Save the y pos, draw later

    DrawCustomText("Distance", elementX, elementY(text), TEXT_HEIGHT, BLACK);
    GuiSliderBar(Rectangle{.x = elementX,
                           .y = static_cast<float>(elementY(slider)),
                           .width = elementWidth,
                           .height = SLIDER_HEIGHT}, 
                           nullptr, nullptr, &objDistance, 0.F, 1.F);

    DrawCustomText("Time", elementX, elementY(text), TEXT_HEIGHT, BLACK);
    GuiSliderBar(Rectangle{.x = elementX,
                           .y = static_cast<float>(elementY(slider)),
                           .width = elementWidth,
                           .height = SLIDER_HEIGHT}, 
                           nullptr, nullptr, &objTime, 0.F, 1.F);

    DrawCustomText("Scenery", elementX, elementY(text), TEXT_HEIGHT, BLACK);
    GuiSliderBar(Rectangle{.x = elementX,
                           .y = static_cast<float>(elementY(slider)),
                           .width = elementWidth,
                           .height = SLIDER_HEIGHT}, 
                           nullptr, nullptr, &objScenery, 0.F, 1.F);

    DrawCustomText("Tourism", elementX, elementY(text), TEXT_HEIGHT, BLACK);
    GuiSliderBar(Rectangle{.x = elementX,
                           .y = static_cast<float>(elementY(slider)),
                           .width = elementWidth,
                           .height = SLIDER_HEIGHT}, 
                           nullptr, nullptr, &objTourism, 0.F, 1.F);

    DrawCustomText("Comfort", elementX, elementY(text), TEXT_HEIGHT, BLACK);
    GuiSliderBar(Rectangle{.x = elementX,
                           .y = static_cast<float>(elementY(slider)),
                           .width = elementWidth,
                           .height = SLIDER_HEIGHT}, 
                           nullptr, nullptr, &objComfort, 0.F, 1.F);

    // Draw the model dropdown last so it's drawn over the sliders if open

    int modelSelectionIndex = static_cast<int>(modelSelection);
    if (GuiDropdownBox(Rectangle{.x = elementX,
                                 .y = static_cast<float>(modelSelectionY),
                                 .width = elementWidth,
                                 .height = BOX_HEIGHT},
                                 "Dijkstra;A Star", &modelSelectionIndex,
                                 modelDropdownEdit) != 0) 
    {
        modelDropdownEdit = !modelDropdownEdit;
        modelSelection = static_cast<PathfindingModel>(modelSelectionIndex);
    }
}

void UserInterface::DrawDebugInfo(const OSMGraph &graph, const OSMRenderer &renderer) 
{
    // Skip drawing if debug is hidden

    if (!showDebug) {
        return;
    }

    // Similar definition as the ones in DrawUserInterface() and DrawRouteInfo()

    const auto debX = uiRect.width - H_PAD.second;
    const auto debY = (screenY - UI_DEBUG_SIZE) - H_PAD.first;
    const auto debWidth  = uiRect.width;
    const auto debHeight = UI_DEBUG_SIZE;

    const Rectangle debRect = {.x = debX, .y = debY, .width = debWidth, .height = debHeight};

    const auto elementX = debRect.x + H_PAD.first;
    const auto elementWidth = uiRect.width + H_PAD.second;

    auto accumulator = debRect.y;
    auto elementY = [&](int type) {
        int current = accumulator;
        accumulator += type;
        return current;
    };

    // The begginning of the debug panel

    GuiPanel(debRect, nullptr);
    
    elementY(padding);
    DrawCustomText("DEBUG INFO", elementX, elementY(heading), HEADING_HEIGHT, BLACK);
    DrawRectangle(elementX, elementY(padding), elementWidth, 1, GRAY);

    DrawCustomText(std::format("FPS: {}", GetFPS()).c_str(), elementX, elementY(text), TEXT_HEIGHT, BLACK);
    DrawCustomText(std::format("Rendered ways: {}", renderer.GetWayRenderCount()).c_str(), elementX, elementY(text), TEXT_HEIGHT, BLACK);
    DrawCustomText(std::format("Total nodes: {}", graph.GetNodeCount()).c_str(), elementX, elementY(text), TEXT_HEIGHT, BLACK);
    DrawCustomText(std::format("Total ways: {}", graph.GetWayCount()).c_str(), elementX, elementY(text), TEXT_HEIGHT, BLACK);
}