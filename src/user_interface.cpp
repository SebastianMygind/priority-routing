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

// Height of elements, effectively their size
constexpr int HEADING_HEIGHT = 30;
constexpr int TEXT_HEIGHT = 20;
constexpr int BOX_HEIGHT = 40;
constexpr int SLIDER_HEIGHT = 20;

// Padding between elements, horizontal and vertical
constexpr std::pair<int, int> H_PAD = {10, -20};
constexpr int V_PAD_S = 5;
constexpr int V_PAD_M = 20;

constexpr int BOX_BORDER_WIDTH = 2;


constexpr int DEBUG_TEXT_TITLE_SIZE = 32;
constexpr int DEBUG_ENTRY_TEXT_SIZE = 20;



// We need to set a minimum size for UI elements which will be done "globally"
// here
constexpr std::pair MIN_UI_SIZE = {200.F, 600.F};

void DrawCursor(const Rectangle &uiRect,
                const Vector2 &mWorldPos,
                Vector2 mPos);

void DrawRouteInfo(const Rectangle &uiRect,
                   const Vector2 &mPos,
                   UIState &state,
                   bool &globalKeyIsLocked);

void DrawDebugInfo(const Rectangle &uiRect,
                   const Window &window,
                   const OSMGraph &graph,
                   const OSMRenderer &renderer);

void DrawUserInterface(const Window &window,
                       const Vector2 &mWorldPos,
                       const OSMGraph &graph,
                       const OSMRenderer &renderer,
                       UIState &state,
                       bool &globalKeyIsLocked) {
    const auto screenX = static_cast<float>(window.width);
    const auto screenY = static_cast<float>(window.height);

    // Only fetch mouse position once for UI logic, might save some compute.
    const auto mPos = GetMousePosition();

    const auto winStartX = screenX * BORDER_SPACING;
    const auto winStartY = screenY * BORDER_SPACING;
    const auto winLenX = std::max(screenX * WINDOW_WIDTH, MIN_UI_SIZE.first);
    const auto winLenY =
        std::max(screenY - (2 * winStartY), MIN_UI_SIZE.second);

    const Rectangle uiRect = {
        .x = winStartX, .y = winStartY, .width = winLenX, .height = winLenY};

    GuiPanel(uiRect, nullptr);

    // These elements make up the UI and must not overlap as that will cause
    // collisions.
    DrawCursor(uiRect, mWorldPos, mPos);
    DrawDebugInfo(uiRect, window, graph, renderer);
    DrawRouteInfo(uiRect, mPos, state, globalKeyIsLocked);
}

// Only draw the cursor if we are not hovering over UI elements.
void DrawCursor(const Rectangle &uiRect,
                const Vector2 &mWorldPos,
                const Vector2 mPos) {
    const Rectangle mRect = {
        .x = mPos.x - 1, .y = mPos.y - 1, .width = 2, .height = 2};

    if (CheckCollisionRecs(uiRect, mRect)) {
        return;
    }

    DrawCircleV(mPos, 4, DARKGRAY);

    DrawTextEx(
        GetFontDefault(), TextFormat("[%.2f, %.2f]", mWorldPos.x, mWorldPos.y),
        Vector2Add(GetMousePosition(), {.x = -44, .y = -24}), 20, 2, BLACK);
}

void DrawDebugInfo(const Rectangle &uiRect,
                   const Window &window,
                   const OSMGraph &graph,
                   const OSMRenderer &renderer) {
    if (!window.showDebug) {
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

// Set up the accumulator used to space out the UI elements
// Calling the accumulator returns an element's y pos and sets up next element's y pos

enum AccumulatorTypes {heading = HEADING_HEIGHT + V_PAD_S, 
                       text = TEXT_HEIGHT + V_PAD_S,
                       box = BOX_HEIGHT + V_PAD_S,
                       slider = SLIDER_HEIGHT + V_PAD_S,
                       padding = V_PAD_M};

void DrawRouteInfo(const Rectangle &uiRect,
                   const Vector2 &mPos,
                   UIState &state,
                   bool &globalKeyIsLocked) {

    // Define sizes and positions of UI elements

    auto elementX = uiRect.x + H_PAD.first;
    auto elementWidth = uiRect.width + H_PAD.second;
    auto accumulator = uiRect.y + V_PAD_M;
    auto elementY = [&](int type) {
        int current = accumulator;
        accumulator += type;
        return current;
    };

    // Location elements

    DrawCustomText("Location", elementX, elementY(heading), HEADING_HEIGHT, BLACK);
    DrawRectangle(elementX, elementY(padding), elementWidth, 1, GRAY);

    DrawCustomText("Origin", elementX, elementY(text), TEXT_HEIGHT, BLACK);
    textBoxFrom.Update(Rectangle{.x = elementX,
                                 .y = static_cast<float>(elementY(box)),
                                 .width = elementWidth,
                                 .height = BOX_HEIGHT}, 
                                 mPos, globalKeyIsLocked);

    DrawCustomText("Destination", elementX, elementY(text), TEXT_HEIGHT, BLACK);
    textBoxTo.Update(Rectangle{.x = elementX,
                               .y = static_cast<float>(elementY(box)),
                               .width = elementWidth,
                               .height = BOX_HEIGHT}, 
                               mPos, globalKeyIsLocked);

    elementY(padding);

    // Algorithm elements

    DrawCustomText("Algorithm", elementX, elementY(heading), HEADING_HEIGHT, BLACK);
    DrawRectangle(elementX, elementY(padding), elementWidth, 1, GRAY);

    DrawCustomText("Model", elementX, elementY(text), TEXT_HEIGHT, BLACK);
    auto dropdownY = elementY(box); // Save the y pos, draw later

    DrawCustomText("Distance", elementX, elementY(text), TEXT_HEIGHT, BLACK);
    GuiSliderBar(Rectangle{.x = elementX,
                           .y = static_cast<float>(elementY(slider)),
                           .width = elementWidth,
                           .height = SLIDER_HEIGHT}, 
                           "", "", &state.objDistance, 0.F, 1.F);

    DrawCustomText("Time", elementX, elementY(text), TEXT_HEIGHT, BLACK);
    GuiSliderBar(Rectangle{.x = elementX,
                           .y = static_cast<float>(elementY(slider)),
                           .width = elementWidth,
                           .height = SLIDER_HEIGHT}, 
                           "", "", &state.objTime, 0.F, 1.F);

    DrawCustomText("Scenery", elementX, elementY(text), TEXT_HEIGHT, BLACK);
    GuiSliderBar(Rectangle{.x = elementX,
                           .y = static_cast<float>(elementY(slider)),
                           .width = elementWidth,
                           .height = SLIDER_HEIGHT}, 
                           "", "", &state.objScenery, 0.F, 1.F);

    DrawCustomText("Tourism", elementX, elementY(text), TEXT_HEIGHT, BLACK);
    GuiSliderBar(Rectangle{.x = elementX,
                           .y = static_cast<float>(elementY(slider)),
                           .width = elementWidth,
                           .height = SLIDER_HEIGHT}, 
                           "", "", &state.objTourism, 0.F, 1.F);

    DrawCustomText("Comfort", elementX, elementY(text), TEXT_HEIGHT, BLACK);
    GuiSliderBar(Rectangle{.x = elementX,
                           .y = static_cast<float>(elementY(slider)),
                           .width = elementWidth,
                           .height = SLIDER_HEIGHT}, 
                           "", "", &state.objComfort, 0.F, 1.F);

    elementY(padding);

    // Lastly the dropdown box

    int modelSelectionIndex = static_cast<int>(state.modelSelection);
    if (GuiDropdownBox(Rectangle{.x = elementX,
                                 .y = static_cast<float>(dropdownY),
                                 .width = elementWidth,
                                 .height = BOX_HEIGHT},
                                 "Dijkstra;A Star", &modelSelectionIndex,
                                 state.modelDropdownEdit) != 0) 
    {
        state.modelDropdownEdit = !state.modelDropdownEdit;
        state.modelSelection = static_cast<PathfindingModel>(modelSelectionIndex);
    }
}
