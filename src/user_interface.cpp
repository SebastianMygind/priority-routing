#include "user_interface.h"

#include "raylib.h"
#include <algorithm>
#include <format>

#include "raymath.h"
#include "../vendor/raygui.h"
#include "Window.h"

#include "widgets/text_box.h"
#include "widgets/text.h"

static TextBox textBoxFrom;
static TextBox textBoxTo;

// This boolean should tell all the ui if it should ignore any input or not.
// We should not change debug state if pressing d while writing to a textbox.

constexpr float BORDER_SPACING = 0.01;
constexpr float WINDOW_WIDTH = 0.20;

constexpr int TEXT_FIELD_HEIGHT = 40;
constexpr int BOX_BORDER_WIDTH = 2;

constexpr int TEXT_TITLE_SIZE = 28;
constexpr int DEBUG_TEXT_TITLE_SIZE = 32;
constexpr int DEBUG_ENTRY_TEXT_SIZE = 20;

constexpr int UI_PADDING_S = 5;
constexpr int UI_PADDING_M = 10;
constexpr int UI_PADDING_L = 30;

// We need to set a minimum size for UI elements which will be done "globally"
// here
constexpr std::pair MIN_UI_SIZE = {200.F, 600.F};

void DrawCursor(const Rectangle &uiRect,
                const Vector2 &mWorldPos,
                Vector2 mPos);

void DrawRouteInfo(const Rectangle &uiRect,
                   const Vector2 &mPos,
                   const OSMGraph &graph,
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
    DrawRouteInfo(uiRect, mPos, graph, state, globalKeyIsLocked);
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

void DrawRouteInfo(const Rectangle &uiRect,
                   const Vector2 &mPos,
                   const OSMGraph &graph,
                   UIState &state,
                   bool &globalKeyIsLocked) {
    // Height accumulator for UI elements
    auto heightAccumulator = static_cast<int>(uiRect.y) + (2 * UI_PADDING_M);

    // Origin TextBox
    DrawCustomText("Origin", static_cast<int>(uiRect.x) + (UI_PADDING_M),
                   heightAccumulator, TEXT_TITLE_SIZE, BLACK);
    heightAccumulator += UI_PADDING_L;

    const Rectangle fromRect = {.x = uiRect.x + UI_PADDING_M,
                                .y = static_cast<float>(heightAccumulator),
                                .width = uiRect.width - (2 * UI_PADDING_M),
                                .height = TEXT_FIELD_HEIGHT};

    heightAccumulator += (UI_PADDING_M * 2) + TEXT_FIELD_HEIGHT;

    textBoxFrom.Update(fromRect, mPos, globalKeyIsLocked);

    // Destination TextBox

    DrawCustomText("Destination", static_cast<int>(uiRect.x) + (UI_PADDING_M),
                   heightAccumulator, TEXT_TITLE_SIZE, BLACK);

    heightAccumulator += UI_PADDING_L;

    const Rectangle toRect = {.x = uiRect.x + UI_PADDING_M,
                              .y = static_cast<float>(heightAccumulator),
                              .width = uiRect.width - (UI_PADDING_M * 2),
                              .height = TEXT_FIELD_HEIGHT};

    textBoxTo.Update(toRect, mPos, globalKeyIsLocked);
    // Model selection dropdown
    // DrawCustomText("Model/Algorithm", static_cast<int>(toRect.x) + 5,
    //               static_cast<int>(uiRect.y));

    heightAccumulator += (UI_PADDING_M * 2) + TEXT_FIELD_HEIGHT;

    DrawCustomText("Algorithm", static_cast<int>(uiRect.x) + (UI_PADDING_M),
                   heightAccumulator, TEXT_TITLE_SIZE, BLACK);

    heightAccumulator += UI_PADDING_L;

    int modelSelectionIndex = static_cast<int>(state.modelSelection);
    if (GuiDropdownBox(Rectangle{.x = uiRect.x + UI_PADDING_M,
                                 .y = static_cast<float>(heightAccumulator),
                                 .width = uiRect.width - 20,
                                 .height = TEXT_FIELD_HEIGHT},
                       "Dijkstra;A Star", &modelSelectionIndex,
                       state.modelDropdownEdit) != 0) {
        state.modelDropdownEdit = !state.modelDropdownEdit;
        state.modelSelection =
            static_cast<PathfindingModel>(modelSelectionIndex);
    }
}
