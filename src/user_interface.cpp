#include "user_interface.h"

#include "raylib.h"
#include <algorithm>
#include <format>

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

UserInterface::~UserInterface() {
    UnloadTexture(spinnerTexture);
    UnloadImage(spinnerImage);
    UnloadTexture(checkTexture);
}

/*
    Keeps track of and increses the y position of the elements based on their type.
*/
float UserInterface::elementY(const int type) {
    const auto current = accumulator;
    accumulator += type;
    return current;
};

/*
    The following functions are shortened "presets" for custom styled raygui elements.
*/
void UserInterface::DrawCustomHeading(const char* text)
{
    elementY(paddingType);

    const Vector2 headingPos{
        .x = elementX,
        .y = elementY(headingType)
    };

    DrawTextEx(fontHeading, text, headingPos, HEADING_HEIGHT, 1.2, BLACK);
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

    const Rectangle textboxPos{
        .x = elementX,
        .y = yPos,
        .width = elementWidth - BOX_HEIGHT,
        .height = BOX_HEIGHT
    };

    const Rectangle outlinePos{
        .x = elementX,
        .y = yPos,
        .width = elementWidth,
        .height = BOX_HEIGHT
    };

    const Rectangle buttonPos{
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
    if (GuiButton(buttonPos, "#113#") != 0)
    {
        text[0] = '\0';
        wasPreviouslyEditing = true;
    }
}

void UserInterface::DrawCustomSelection(const char* text, float posY, int* selection, bool& edit) {
    const Rectangle selectionPos{
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
    const Rectangle sliderPos{
        .x = elementX,
        .y = elementY(sliderType),
        .width = elementWidth,
        .height = SLIDER_HEIGHT
    };

    GuiSliderBar(sliderPos, nullptr, nullptr, value, 0.F, 1.F);
}

void UserInterface::DrawCustomPather(int& index, int count)
{
    auto const yPos = elementY(boxType);

    const Rectangle outlinePos{
        .x = elementX,
        .y = yPos,
        .width = elementWidth,
        .height = BOX_HEIGHT
    };

    const Vector2 textPos{
        .x = elementX + 56,
        .y = yPos + 10
    };

    const Rectangle leftButton{
        .x = elementX,
        .y = yPos,
        .width = BOX_HEIGHT,
        .height = BOX_HEIGHT
    };

    const Rectangle rightButton{
        .x = elementX + elementWidth - BOX_HEIGHT,
        .y = yPos,
        .width = BOX_HEIGHT,
        .height = BOX_HEIGHT
    };

    const std::string text = (count > 0)
        ? std::format("Route {} of {}", index + 1, count  + 1)
        : "  No routes";

    DrawRectangleLinesEx(outlinePos, 1, GRAY);
    DrawTextEx(fontText, text.c_str(), textPos, TEXT_HEIGHT, 1.2, BLACK);

    if (GuiButton(leftButton, "#114#") != 0)
    {
        index = std::max(0, index - 1);
    }
    if (GuiButton(rightButton, "#115#") != 0 && count > 0)
    {
        index = std::min(count - 1, index + 1);
    }
}

/*
    Sets the fonts needed for the UI and sets the proper raygui style.
*/
void UserInterface::SetupUI(const char* file)
{ 
    fontText = LoadFontEx(file, TEXT_HEIGHT, nullptr, 0);
    fontHeading = LoadFontEx(file, HEADING_HEIGHT, nullptr, 0);
    GuiSetFont(fontText);
    GuiSetStyle(DEFAULT, TEXT_SIZE, TEXT_HEIGHT);
    GuiSetStyle(DEFAULT, BORDER_WIDTH, 1);
    GuiSetStyle(TEXTBOX, BORDER_WIDTH, 0);

    spinnerImage = LoadImageAnim("../assets/spinner.gif", &spinnerFrameCount);
    spinnerTexture = LoadTextureFromImage(spinnerImage);

    auto checkImage = LoadImage("../assets/checkmark.png");
    ImageResize(&checkImage, 50, 50);
    checkTexture = LoadTextureFromImage(checkImage);

    UnloadImage(checkImage);
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
    DrawPathLoader(window);
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

    switch (modelSelectionIndex)
    {
    case 2: // Weighted Sum
        DrawCustomText("Distance");
        DrawCustomSlider(&objDistance);
        DrawCustomText("Time");
        DrawCustomSlider(&objTime);
        DrawCustomText("Lit Roads");
        DrawCustomSlider(&objLitRoads);
        DrawCustomText("Smoothness");
        DrawCustomSlider(&objSmoothness);
        DrawCustomText("Gas Station");
        DrawCustomSlider(&objGasStation);
        DrawCustomText("Cafe");
        DrawCustomSlider(&objCafe);
        DrawCustomText("Tourism");
        DrawCustomSlider(&objTourism);
        break;
    
    case 3:
        DrawCustomPather(pathSelectionIndex, pathCount);

    default:
        break;
    }
    

    // Draw the model dropdown last so it's drawn over the sliders if open
    DrawCustomSelection("Dijkstra;A Star;Weighted Sum", modelY, &modelSelectionIndex, modelSelectionEdit);

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

    // The beginning of the debug panel

    GuiPanel(debRect, nullptr);
    
    DrawCustomHeading("DEBUG");
    DrawCustomText(std::format("FPS: {}", GetFPS()).c_str());
    DrawCustomText(std::format("Coords: {:.3f}, {:.3f}", debugMouseWorldPos.x, debugMouseWorldPos.y).c_str());
    DrawCustomText(std::format("Nodes: {}/{}", debugRenderNodes, debugTotalNodes).c_str());
    DrawCustomText(std::format("Ways: {}/{}", debugRenderedWays, debugTotalWays).c_str());
    DrawCustomText(std::format("Model Time: {:.3f} ms", debugModelTime.count()).c_str());
    DrawCustomText(std::format("POI: {}", poiText).c_str());

    // Mouse crosshair

    DrawRectangle(mousePos.x-20, mousePos.y-1, 40, 2, DARKGRAY);
    DrawRectangle(mousePos.x-1, mousePos.y-20, 2, 40, DARKGRAY);
}

void UserInterface::DrawPathLoader(const Window &window) {
    constexpr auto displayHeight = 50.;
    constexpr auto displayWidth = 50.;

    const auto xPos = window.width - displayWidth - 50;
    constexpr auto yPos = 50;

    if (pathfindingInProgress) {
        if (spinnerFrameTimer.IsActive()) {
            currentFrame += 1;
            if (currentFrame >= spinnerFrameCount) currentFrame = 0;

            const auto nextFrameDataOffset = spinnerImage.width*spinnerImage.height*4*currentFrame;
            UpdateTexture(spinnerTexture, (static_cast<unsigned char *>(spinnerImage.data) + nextFrameDataOffset));
        }
        const Rectangle sourceRec = {
            .x=0.0F,
            .y=0.0F,
            .width=static_cast<float>(spinnerTexture.width),
            .height=static_cast<float>(spinnerTexture.height)
        };
        const Rectangle destRec = {
            .x=static_cast<float>(xPos),
            .y=yPos,
            .width=displayWidth,
            .height=displayHeight
        };

        constexpr Vector2 origin = { .x=0.0F, .y=0.0F };

        DrawTexturePro(spinnerTexture, sourceRec, destRec, origin, 0.0F, WHITE);
    }

    const auto passedTime = std::chrono::steady_clock::now() - lastCompletion;
    if (passedTime >= showTime) {
        return;
    }

    const float progress = std::chrono::duration<float>(passedTime) / std::chrono::duration<float>(showTime);
    const float alpha = 1.0F - progress;

    DrawTexture(checkTexture, xPos, yPos, Fade(WHITE, alpha));

}

void UserInterface::ActivateLoader() {
    pathfindingInProgress = true;
}

void UserInterface::DeactivateLoader() {
    pathfindingInProgress = false;
    lastCompletion = std::chrono::steady_clock::now();
}

/*
    A locked UI means that the mouse is interacting with the map, so the UI
   elements cannot be interacted with. This function should be called before
   input handling and checks if the mouse is clicking inside the UI. MouseInUI()
   would therefore signal if the mouse is focused on the UI (if it's left
   unlocked).
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