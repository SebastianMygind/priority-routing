#include "user_interface.h"

#include "raylib.h"
#include <algorithm>
#include <format>

#include "raymath.h"
#include "Window.h"

#include "spdlog/spdlog.h"

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

/*
    Accumulator types are used with elementY to determine the y position of the next element.
    Use the type of the element you're drawing or call elementY(padding) to add spacing between elements.
*/
enum AccumulatorTypes 
{
    headingType = HEADING_HEIGHT + V_PAD, 
    textType = TEXT_HEIGHT + V_PAD,
    boxType = BOX_HEIGHT + V_PAD,
    sliderType = SLIDER_HEIGHT + V_PAD,
    paddingType = 4 * V_PAD
};

/*
    Keeps track of and increses the y position of the elements based on their type.
*/
float UserInterface::elementY(int type) {
    auto current = accumulator;
    accumulator += type;
    return current;
};

/*
    The following functions are shortened "presets" for custom styled raygui elements.
*/
void UserInterface::DrawCustomHeading(const char* heading)
{
    elementY(paddingType);

    Vector2 headingPos{
        .x = elementX,
        .y = elementY(headingType)
    };

    DrawTextEx(fontHeading, heading, headingPos, HEADING_HEIGHT, 1.2, BLACK);
    DrawRectangle(elementX, elementY(paddingType), elementWidth, 1, GRAY);
}

void UserInterface::DrawCustomText(const char* text)
{
    Vector2 textPos{
        .x = elementX,
        .y = elementY(textType)
    };

    DrawTextEx(fontText, text, textPos, TEXT_HEIGHT, 1.2, BLACK);

}

void UserInterface::DrawCustomTextbox(char* text, bool& edit)
{
    auto const yPos = elementY(boxType);

    Rectangle textboxPos{
        .x = elementX,
        .y = yPos,
        .width = elementWidth - BOX_HEIGHT,
        .height = BOX_HEIGHT
    };

    Rectangle outlinePos{
        .x = elementX,
        .y = yPos,
        .width = elementWidth,
        .height = BOX_HEIGHT
    };

    Rectangle buttonPos{
        .x = elementWidth - BOX_HEIGHT - H_PAD.second,
        .y = yPos,
        .width = BOX_HEIGHT,
        .height = BOX_HEIGHT
    };

    if (GuiTextBox(textboxPos, text, textboxSize, edit) != 0)
    {
        edit = !edit;
    }
    DrawRectangleLinesEx(outlinePos, 1, GRAY);
    if (GuiButton(buttonPos, "#113#"))
    {
        text[0] = '\0';
        wasPreviouslyEditing = true;
    }
}

void UserInterface::DrawCustomSelection(const char* text, float posY, int* selection, bool& edit)
{
    Rectangle selectionPos{
        .x = elementX,
        .y = posY,
        .width = elementWidth,
        .height = BOX_HEIGHT
    };

    if (GuiDropdownBox(selectionPos, text, selection, edit) != 0) 
    {
        edit = !edit;
    }
}

void UserInterface::DrawCustomSlider(float* value)
{
    Rectangle sliderPos{
        .x = elementX,
        .y = elementY(sliderType),
        .width = elementWidth,
        .height = SLIDER_HEIGHT
    };

    GuiSliderBar(sliderPos, nullptr, nullptr, value, 0.F, 1.F);
}

/*
    Sets the fonts needed for the UI and sets the proper raygui style.
*/
void UserInterface::SetupFontConfig(const char* file) 
{ 
    fontText = LoadFontEx(file, TEXT_HEIGHT, nullptr, 0);
    fontHeading = LoadFontEx(file, HEADING_HEIGHT, nullptr, 0);
    GuiSetFont(fontText);
    GuiSetStyle(DEFAULT, TEXT_SIZE, TEXT_HEIGHT);
    GuiSetStyle(DEFAULT, BORDER_WIDTH, 1);
    GuiSetStyle(TEXTBOX, BORDER_WIDTH, 0);
}

/*
    The main UI function, it creates a rectangle based on the screen size and then calls
    the routes and debug functions that use the presets from above to draw the UI. 
*/
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

    elementX = uiRect.x + H_PAD.first;
    elementWidth = uiRect.width + H_PAD.second;
    accumulator = uiRect.y;

    // The beginning of the ui panel
    
    GuiPanel(uiRect, nullptr);

    DrawCustomHeading("Location");
    DrawCustomText("Origin");
    DrawCustomTextbox(originTextboxText, originTextboxEdit);
    DrawCustomText("Destination");
    DrawCustomTextbox(destinationTextboxText, destinationTextboxEdit);

    DrawCustomHeading("Algorithm");
    DrawCustomText("Model");
    // Save the y pos, draw later
    float modelY = elementY(boxType); 
    DrawCustomText("Distance");
    DrawCustomSlider(&objDistance);
    DrawCustomText("Time");
    DrawCustomSlider(&objTime);
    DrawCustomText("Gas Station");
    DrawCustomSlider(&objScenery);
    DrawCustomText("Cafe");
    DrawCustomSlider(&objTourism);
    DrawCustomText("Comfort");
    DrawCustomSlider(&objComfort);

    // Draw the model dropdown last so it's drawn over the sliders if open
    DrawCustomSelection("Dijkstra;A Star", modelY, &modelSelectionIndex, modelSelectionEdit);

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

    elementX = debRect.x + H_PAD.first;
    elementWidth = uiRect.width + H_PAD.second;
    accumulator = debRect.y;

    // The begginning of the debug panel

    GuiPanel(debRect, nullptr);
    
    DrawCustomHeading("DEBUG");
    DrawCustomText(std::format("FPS: {}", GetFPS()).c_str());
    DrawCustomText(std::format("Coords: {:.3f}, {:.3f}", debugMouseWorldPos.x, debugMouseWorldPos.y).c_str());
    DrawCustomText(std::format("Nodes: {}/{}", debugRenderNodes, debugTotalNodes).c_str());
    DrawCustomText(std::format("Ways: {}/{}", debugRenderedWays, debugTotalWays).c_str());
    DrawCustomText(std::format("Model Time: {:.3f} ms", debugModelTime.count()).c_str());

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
        modelSelectionEdit = false;
    }
    // If the left mouse is released, unlock it
    if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT))
    {
        GuiUnlock();
    }
}

/* 
    A function that triggers only once after modifying values in the UI.
    Can be manually triggered by setting wasPreviouslyEditing = true. 
    Meant to be used in the main loop.
*/
bool UserInterface::IsUpdated()
{
    const bool isCurrentlyEditing = (originTextboxEdit || destinationTextboxEdit || modelSelectionEdit);
    
    if (!isCurrentlyEditing && wasPreviouslyEditing) 
    {
        wasPreviouslyEditing = false;
        return true;
    }
    
    wasPreviouslyEditing = isCurrentlyEditing;
    return false;
}