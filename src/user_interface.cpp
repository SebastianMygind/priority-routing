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

// This boolean should tell all the ui if it should ignore any input or not.
// We should not change debug state if pressing d while writing to a textbox.

constexpr float BORDER_SPACING = 0.01;
constexpr float WINDOW_WIDTH = 0.20;

constexpr int BOX_BORDER_WIDTH = 2;


constexpr int DEBUG_TEXT_TITLE_SIZE = 32;
constexpr int DEBUG_ENTRY_TEXT_SIZE = 20;


// We need to set a minimum size for UI elements which will be done "globally"
// here
constexpr std::pair MIN_UI_SIZE = {200.F, 600.F};


void UserInterface::DrawUserInterface(const Window &window,
                       const Vector2 &mouseWorldPos,
                       const OSMGraph &graph,
                       const OSMRenderer &renderer,
                       bool &globalKeyIsLocked) 
{
    // Skip drawing if UI is hidden

    if (!showUI) {
        return;
    }

    // Set up the UI box dimensions based on the window size

    const auto screenX = static_cast<float>(window.width);
    const auto screenY = static_cast<float>(window.height);

    const auto winX = screenX * BORDER_SPACING;
    const auto winY = screenY * BORDER_SPACING;
    const auto winWidth  = std::max(screenX * WINDOW_WIDTH, MIN_UI_SIZE.first);
    const auto winHeight = std::max(screenY - (2 * winY), MIN_UI_SIZE.second);

    // Set up the UI box and mouse pos used throughout the class

    uiRect = {.x = winX, .y = winY, .width = winWidth, .height = winHeight};
    mousePos = GetMousePosition();

    // Draw the UI

    GuiPanel(uiRect, nullptr);
    DrawRouteInfo(globalKeyIsLocked);
    DrawDebugInfo(graph, renderer);
    DrawCursor(mouseWorldPos);
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

void UserInterface::DrawDebugInfo(
                   const OSMGraph &graph,
                   const OSMRenderer &renderer) {
    if (!showDebug) {
        return;
    }

    const Vector2 startVec = {.x = uiRect.x + (uiRect.width * 0.05F),
                              .y = uiRect.y + (uiRect.height * 0.6F)};

    const Rectangle debugRect = {.x = startVec.x,
                                 .y = startVec.y,
                                 .width = uiRect.width * 0.9F,
                                 .height = uiRect.height * 0.35F};

    const auto currentFPS = GetFPS();

    DrawRectangleRec(debugRect, {.r = 245, .g = 181, .b = 39, .a = 230});
    DrawRectangleLinesEx(debugRect, 2, {.r = 150, .g = 50, .b = 50, .a = 255});

    // Below is the actual debug information, Constant spacing seems better for
    // this so the offsets are in pixels.

    DrawCustomText("DEBUG INFO", static_cast<int>(debugRect.x) + 10,
                   static_cast<int>(debugRect.y) + 10, DEBUG_TEXT_TITLE_SIZE,
                   BLACK);

    DrawCustomText(std::format("FPS: {}", currentFPS).c_str(),
                   static_cast<int>(debugRect.x) + 10,
                   static_cast<int>(debugRect.y) + 50, DEBUG_ENTRY_TEXT_SIZE,
                   BLACK);

    DrawCustomText(
        std::format("Rendered ways: {}", renderer.GetWayRenderCount()).c_str(),
        static_cast<int>(debugRect.x) + 10, static_cast<int>(debugRect.y) + 70,
        DEBUG_ENTRY_TEXT_SIZE, BLACK);

    DrawCustomText(std::format("Total nodes: {}", graph.GetNodeCount()).c_str(),
                   static_cast<int>(debugRect.x) + 10,
                   static_cast<int>(debugRect.y) + 90, DEBUG_ENTRY_TEXT_SIZE,
                   BLACK);

    DrawCustomText(std::format("Total way: {}", graph.GetWayCount()).c_str(),
                   static_cast<int>(debugRect.x) + 10,
                   static_cast<int>(debugRect.y) + 110, DEBUG_ENTRY_TEXT_SIZE,
                   BLACK);
}

// todo! add back positional ui info
// DrawTextEx(
//  GetFontDefault(),
//  TextFormat("Selected Node: \n A:%i, B:%i", graph.selected_node_a,
//  graph.selected_node_b), {10, 10}, 20, 2, BLACK
//  );

// Accumulator types are used with elementY to determine the y position of the next element
// Use the type of the element you're drawing or call elementY(padding) to add spacing between elements

enum AccumulatorTypes 
{
    heading = HEADING_HEIGHT + V_PAD, 
    text = TEXT_HEIGHT + V_PAD,
    box = BOX_HEIGHT + V_PAD,
    slider = SLIDER_HEIGHT + V_PAD,
    padding = 4 * V_PAD
};

void UserInterface::DrawRouteInfo(bool &globalKeyIsLocked) 
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

    // Location elements

    elementY(padding);
    DrawCustomText("Location", elementX, elementY(heading), HEADING_HEIGHT, BLACK);
    DrawRectangle(elementX, elementY(padding), elementWidth, 1, GRAY);

    DrawCustomText("Origin", elementX, elementY(text), TEXT_HEIGHT, BLACK);
    textBoxFrom.Update(Rectangle{.x = elementX,
                                 .y = static_cast<float>(elementY(box)),
                                 .width = elementWidth,
                                 .height = BOX_HEIGHT}, 
                                 mousePos, globalKeyIsLocked);

    DrawCustomText("Destination", elementX, elementY(text), TEXT_HEIGHT, BLACK);
    textBoxTo.Update(Rectangle{.x = elementX,
                               .y = static_cast<float>(elementY(box)),
                               .width = elementWidth,
                               .height = BOX_HEIGHT}, 
                               mousePos, globalKeyIsLocked);

    // Algorithm elements

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

    // Lastly the dropdown box

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

bool UserInterface::MouseInUI() 
{
    return CheckCollisionPointRec(mousePos, uiRect) && showUI;
}