#include "user_interface.h"

#include "raylib.h"
#include <algorithm>
#include <format>

#include "raymath.h"
#include "Window.h"

#include "widgets/text_box.h"
#include "widgets/text.h"
#include "spdlog/spdlog.h"

static TextBox textBoxFrom;
static TextBox textBoxTo;

// Height of elements, effectively their size

constexpr int HEADING_HEIGHT = 30;
constexpr int TEXT_HEIGHT    = 20;
constexpr int BOX_HEIGHT     = 40;
constexpr int SLIDER_HEIGHT  = 20;

// Padding between elements, horizontal and vertical

constexpr std::pair H_PAD = {10, -20};
constexpr int       V_PAD = 5;

// Used to scale the UI accordingly

constexpr std::pair UI_MIN_SIZE   = {200.F, 620.F};
constexpr float     UI_MULTIPLIER = 0.20;
constexpr float     UI_DEBUG_HEIGHT = 250.F;

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

void UserInterface::SetupFontConfig(const char* file) 
{ 
    fontText = LoadFontEx(file, TEXT_HEIGHT, nullptr, 0);
    fontHeading = LoadFontEx(file, HEADING_HEIGHT, nullptr, 0);
    GuiSetFont(fontText);
    GuiSetStyle(DEFAULT, TEXT_SIZE, TEXT_HEIGHT);
}

void UserInterface::DrawUserInterface(const Window &window) 
{
    // Skip drawing if UI is hidden

    if (!showUI) {
        return;
    }
    
    // Set up the UI box dimensions based on the window size

    const auto screenX = static_cast<float>(window.width);
    const auto screenY = static_cast<float>(window.height);

    const auto winX = H_PAD.first;
    const auto winY = H_PAD.first;
    const auto winWidth  = std::max(screenX * UI_MULTIPLIER, UI_MIN_SIZE.first);
    const auto winHeight = std::max(screenY + H_PAD.second, UI_MIN_SIZE.second);

    // Set up the UI box and mouse pos used throughout the class

    uiRect = {.x = winX, .y = winY, .width = winWidth, .height = winHeight};
    mousePos = GetMousePosition();

    // Draw the UI

    DrawRouteInfo();
    DrawDebugInfo();
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
    DrawTextEx(fontHeading, "Location", { .x = elementX, .y = static_cast<float>(elementY(heading)) }, HEADING_HEIGHT, 1.2, BLACK);
    DrawRectangle(elementX, elementY(padding), elementWidth, 1, GRAY);
    
    DrawTextEx(fontText, "Origin", { .x = elementX, .y = static_cast<float>(elementY(text)) }, TEXT_HEIGHT, 1.2, BLACK);
    if (GuiTextBox(Rectangle{.x = elementX,
                       .y = static_cast<float>(elementY(box)),
                       .width = elementWidth,
                       .height = BOX_HEIGHT},
                        originTextboxText, sizeof(originTextboxText), originTextboxEdit) != 0)
    {
        originTextboxEdit = !originTextboxEdit;
    }

    DrawTextEx(fontText, "Destination", { .x = elementX, .y = static_cast<float>(elementY(text)) }, TEXT_HEIGHT, 1.2, BLACK);
    if (GuiTextBox(Rectangle{.x = elementX,
                       .y = static_cast<float>(elementY(box)),
                       .width = elementWidth,
                       .height = BOX_HEIGHT},
                        destinationTextboxText, sizeof(destinationTextboxText), destinationTextboxEdit) != 0)
    {
        destinationTextboxEdit = !destinationTextboxEdit;
    }


    // "Algorithm" elements - heading text, dropdown for model selection and sliders for the different weights

    elementY(padding);
    DrawTextEx(fontHeading, "Algorithm", { .x = elementX, .y = static_cast<float>(elementY(heading)) }, HEADING_HEIGHT, 1.2, BLACK);
    DrawRectangle(elementX, elementY(padding), elementWidth, 1, GRAY);

    DrawTextEx(fontText, "Model", { .x = elementX, .y = static_cast<float>(elementY(text)) }, TEXT_HEIGHT, 1.2, BLACK);
    auto modelSelectionY = elementY(box); // Save the y pos, draw later

    DrawTextEx(fontText, "Distance", { .x = elementX, .y = static_cast<float>(elementY(text)) }, TEXT_HEIGHT, 1.2, BLACK);
    GuiSliderBar(Rectangle{.x = elementX,
                           .y = static_cast<float>(elementY(slider)),
                           .width = elementWidth,
                           .height = SLIDER_HEIGHT}, 
                           nullptr, nullptr, &objDistance, 0.F, 1.F);

    DrawTextEx(fontText, "Time", { .x = elementX, .y = static_cast<float>(elementY(text)) }, TEXT_HEIGHT, 1.2, BLACK);
    GuiSliderBar(Rectangle{.x = elementX,
                           .y = static_cast<float>(elementY(slider)),
                           .width = elementWidth,
                           .height = SLIDER_HEIGHT}, 
                           nullptr, nullptr, &objTime, 0.F, 1.F);

    DrawTextEx(fontText, "Scenery", { .x = elementX, .y = static_cast<float>(elementY(text)) }, TEXT_HEIGHT, 1.2, BLACK);
    GuiSliderBar(Rectangle{.x = elementX,
                           .y = static_cast<float>(elementY(slider)),
                           .width = elementWidth,
                           .height = SLIDER_HEIGHT}, 
                           nullptr, nullptr, &objScenery, 0.F, 1.F);

    DrawTextEx(fontText, "Tourism", { .x = elementX, .y = static_cast<float>(elementY(text)) }, TEXT_HEIGHT, 1.2, BLACK);
    GuiSliderBar(Rectangle{.x = elementX,
                           .y = static_cast<float>(elementY(slider)),
                           .width = elementWidth,
                           .height = SLIDER_HEIGHT}, 
                           nullptr, nullptr, &objTourism, 0.F, 1.F);

    DrawTextEx(fontText, "Comfort", { .x = elementX, .y = static_cast<float>(elementY(text)) }, TEXT_HEIGHT, 1.2, BLACK);
    GuiSliderBar(Rectangle{.x = elementX,
                           .y = static_cast<float>(elementY(slider)),
                           .width = elementWidth,
                           .height = SLIDER_HEIGHT}, 
                           nullptr, nullptr, &objComfort, 0.F, 1.F);

    // Draw the model dropdown last so it's drawn over the sliders if open

    if (GuiDropdownBox(Rectangle{.x = elementX,
                                 .y = static_cast<float>(modelSelectionY),
                                 .width = elementWidth,
                                 .height = BOX_HEIGHT},
                                 "Dijkstra;A Star", &modelSelection,
                                 modelDropdownEdit) != 0) 
    {
        modelDropdownEdit = !modelDropdownEdit;
    }
}

void UserInterface::DrawDebugInfo() 
{
    // Skip drawing if debug is hidden

    if (!showDebug) {
        return;
    }

    // Similar definition as the ones in DrawUserInterface() and DrawRouteInfo()

    const auto debX = uiRect.width - H_PAD.second;
    const auto debY = uiRect.height - UI_DEBUG_HEIGHT + H_PAD.first;
    const auto debWidth  = uiRect.width;
    const auto debHeight = UI_DEBUG_HEIGHT;

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
    DrawTextEx(fontHeading, "Debug", { .x = elementX, .y = static_cast<float>(elementY(heading)) }, HEADING_HEIGHT, 1.2, BLACK);
    DrawRectangle(elementX, elementY(padding), elementWidth, 1, GRAY);

    DrawTextEx(fontText, std::format("FPS: {}", GetFPS()).c_str(), { .x = elementX, .y = static_cast<float>(elementY(text)) }, TEXT_HEIGHT, 1.2, BLACK);
    DrawTextEx(fontText, std::format("Coords: {:.3f}, {:.3f}", debugMouseWorldPos.x, debugMouseWorldPos.y).c_str(), { .x = elementX, .y = static_cast<float>(elementY(text)) }, TEXT_HEIGHT, 1.2, BLACK);
    DrawTextEx(fontText, std::format("Nodes: {}/{}", debugRenderNodes, debugTotalNodes).c_str(), { .x = elementX, .y = static_cast<float>(elementY(text)) }, TEXT_HEIGHT, 1.2, BLACK);
    DrawTextEx(fontText, std::format("Ways: {}/{}", debugRenderedWays, debugTotalWays).c_str(), { .x = elementX, .y = static_cast<float>(elementY(text)) }, TEXT_HEIGHT, 1.2, BLACK);
    DrawTextEx(fontText, std::format("Model Time: {:.3f} ms", debugModelTime.count()).c_str(), { .x = elementX, .y = static_cast<float>(elementY(text)) }, TEXT_HEIGHT, 1.2, BLACK);

    // Mouse crosshair

    DrawRectangle(mousePos.x-20, mousePos.y-1, 40, 2, DARKGRAY);
    DrawRectangle(mousePos.x-1, mousePos.y-20, 2, 40, DARKGRAY);
}


/* 
    A locked UI means that the mouse is interacting with the map, so the UI elements cannot be interacted with.
    This function should be called before input handling and checks if the mouse is clicking inside the UI.
    MouseInUI() would therefore signal if the mouse is focused on the UI (if it's left unlocked).
*/
void UserInterface::UpdateLockState() 
{
    // If the left mouse is pressed when the cursor is outside the visible UI box, lock it
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && !(CheckCollisionPointRec(mousePos, uiRect) && showUI))
    {
        GuiLock();
        originTextboxEdit = false;
        destinationTextboxEdit = false;
        modelDropdownEdit = false;
    }
    // If the left mouse is released, unlock it
    if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT))
    {
        GuiUnlock();
    }
}

bool UserInterface::IsUpdated()
{
    const bool isCurrentlyEditing = (originTextboxEdit || destinationTextboxEdit || modelDropdownEdit);
    
    if (!isCurrentlyEditing && wasPreviouslyEditing) 
    {
        wasPreviouslyEditing = false;
        return true;
    }
    
    wasPreviouslyEditing = isCurrentlyEditing;
    return false;
}