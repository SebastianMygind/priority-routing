#include "user_interface.h"

#include "raylib.h"
#include <algorithm>
#include <format>

#include "Window.h"
#include "osm/graph.h"
#include "spdlog/spdlog.h"

// Height of elements, effectively their size

constexpr int HEADING_HEIGHT = 30;
constexpr int TEXT_HEIGHT    = 15;
constexpr int BOX_HEIGHT     = 25;
constexpr int SLIDER_HEIGHT  = 15;

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
    textType = (TEXT_HEIGHT - 4) + V_PAD,
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

    const std::string text = (count > 0)
        ? std::format("Route {} of {}", index + 1, count)
        : "No routes";

    Vector2 textSize = MeasureTextEx(fontText, text.c_str(), TEXT_HEIGHT, 1.2);

    const Rectangle outlinePos{
        .x = elementX,
        .y = yPos,
        .width = elementWidth,
        .height = BOX_HEIGHT
    };

    const Vector2 textPos{
        .x = elementX + (elementWidth / 2) - (textSize.x / 2),
        .y = yPos + (BOX_HEIGHT / 2) - (TEXT_HEIGHT / 2) + 1
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



    DrawRectangleLinesEx(outlinePos, 1, GRAY);
    DrawTextEx(fontText, text.c_str(), textPos, TEXT_HEIGHT, 1.2, BLACK);

    if (GuiButton(leftButton, "#114#") != 0)
    {
        index = std::max(0, index - 1);
        pathSelectionCallback(index);
    }
    if (GuiButton(rightButton, "#115#") != 0 && count > 0)
    {
        index = std::min(count - 1, index + 1);
        pathSelectionCallback(index);
    }
}

void UserInterface::DrawObjecive(ObjectiveList& objectives, int index, bool removable) 
{
    Objective& objective = objectives[index];

    auto const yPos = elementY(boxType);

    Vector2 textSize = MeasureTextEx(fontText, objective.name.c_str(), TEXT_HEIGHT, 1.2);

    const Rectangle outlinePos{
        .x = elementX,
        .y = yPos,
        .width = elementWidth,
        .height = BOX_HEIGHT
    };

    const Vector2 textPos{
        .x = elementX + 10,
        .y = yPos + (BOX_HEIGHT / 2) - (TEXT_HEIGHT / 2) + 1
    };

    const Rectangle rightButton{
        .x = elementX + elementWidth - BOX_HEIGHT + 5,
        .y = yPos + 5,
        .width = BOX_HEIGHT - 10,
        .height = BOX_HEIGHT - 10
    };

    DrawRectangleLinesEx(outlinePos, 1, LIGHTGRAY);
    DrawTextEx(fontText, objective.name.c_str(), textPos, TEXT_HEIGHT, 1.2, BLACK);

    if (removable) 
    {
        if (GuiButton(rightButton, "x") != 0)
        {
            objectives.erase(objectives.begin() + index);
        }
    }
}

void UserInterface::DrawObjeciveWeight(ObjectiveList& objectives, int index, bool removable) 
{
    Objective& objective = objectives[index];

    auto const yPos = elementY(boxType);

    Vector2 textSize = MeasureTextEx(fontText, objective.name.c_str(), TEXT_HEIGHT, 1.2);

    const Rectangle outlinePos{
        .x = elementX,
        .y = yPos,
        .width = elementWidth,
        .height = BOX_HEIGHT
    };

    const Vector2 textPos{
        .x = elementX + 10,
        .y = yPos + (BOX_HEIGHT / 2) - (TEXT_HEIGHT / 2) + 1
    };

    const Rectangle rightButton{
        .x = elementX + elementWidth - BOX_HEIGHT + 5,
        .y = yPos + 5,
        .width = BOX_HEIGHT - 10,
        .height = BOX_HEIGHT - 10
    };

    DrawRectangleLinesEx(outlinePos, 1, LIGHTGRAY);
    DrawTextEx(fontText, objective.name.c_str(), textPos, TEXT_HEIGHT, 1.2, BLACK);

    if (removable) 
    {
        if (GuiButton(rightButton, "x") != 0)
        {
            objectives.erase(objectives.begin() + index);
        }
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

    objPareto = {
        { "Distance", DistanceObjective, 0.0 },
        { "Time", TravelTimeObjective, 0.0 },
        { "Traffic Lights", TrafficSignalObjective, 0.0 },
    };

    objWeightedSum =  {
        { "Distance", DistanceObjective, 0.5 },
        { "Time", TravelTimeObjective, 0.5 },
        { "Lit Roads", LitRoadObjective, 0.5 },
        { "Road Smoothness", RoadSmoothnessObjective, 0.5 },
        
    };

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
    const auto winWidth  = std::min(std::max(screenX * UI_MULTIPLIER, UI_MIN_SIZE.first), 400.f);
    const auto winHeight = std::max(screenY + H_PAD.second, UI_MIN_SIZE.second);

    // Set up the UI box and mouse pos used throughout the class

    uiRect = {.x = winX, .y = winY, .width = winWidth, .height = winHeight};
    mousePos = GetMousePosition();

    // Draw the UI

    DrawRouteInfo();
    DrawPathLoader(window);
    DrawDebugInfo();


    static int scroll = 0;
    GuiListView({ .x = elementX, .y = 10, .width = elementWidth, .height = 100 },
        "Distance;Time;Lit Roads;Smoothness;Gas Station;Cafe;Tourism",
        &scroll,
        nullptr);
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

    DrawCustomText("Objectives");
    switch (modelSelectionIndex)
    {
        case 0:  // Dijkstra
        case 1:
        //DrawObjecive("Distance", false, 0);

        break;

    case 2: // Weighted Sum
        for (size_t i = 0; i < objWeightedSum.size(); i++) 
        {
            DrawObjeciveWeight(objWeightedSum, i, true);
        }

        GuiButton({.x = elementX,
                   .y = elementY(boxType),
                   .width = elementWidth,
                   .height = BOX_HEIGHT},
                  "Add");
        break;
    
    case 3:
    {
        for (size_t i = 0; i < objPareto.size(); i++) 
        {
            DrawObjecive(objPareto, i, true);
        }

        GuiButton({.x = elementX,
                   .y = elementY(boxType),
                   .width = elementWidth,
                   .height = BOX_HEIGHT},
                  "Add");
        break;
    }

    default:
        break;
    }
    
    DrawCustomHeading("Solution");
    DrawCustomPather(pathSelectionIndex, pathCount);

    // Draw the model dropdown last so it's drawn over the sliders if open
    DrawCustomSelection("Dijkstra;A Star;Weighted Sum;Pareto", modelY, &modelSelectionIndex, modelSelectionEdit);


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