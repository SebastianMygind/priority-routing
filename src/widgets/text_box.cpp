#include "text_box.h"

#include <raylib.h>

#include "text.h"

constexpr int FONT_SIZE = 16;
constexpr float TEXT_SPACING = 2.0F;

const Font FONT = GetFontDefault();

void TextBox::DrawTextBox(const Rectangle bounds) const {


    auto borderColor = BLACK;
    Color surfaceColor = {.r = 230, .g = 230, .b = 230, .a = 255};
    if (isEditing) {
        borderColor = {.r = 55, .g = 55, .b = 200, .a = 255};
        surfaceColor = WHITE;
    }

    DrawRectangleRec(bounds, surfaceColor);
    DrawRectangleLinesEx(bounds, 1, borderColor);

    const Vector2 textSize = MeasureTextEx(
        FONT,
        this->buffer.c_str(),
        FONT_SIZE,
        TEXT_SPACING
    );

    Vector2 textPos;
    textPos.x = bounds.x + 5;
    textPos.y = bounds.y + 10;


    DrawCustomText(buffer.c_str(), textPos.x, textPos.y, 28, BLACK);
}

void TextBox::UpdateEditMode(const Rectangle bounds, const Vector2 &mPos, bool &globalIsEditing) {
    if (CheckCollisionCircleRec(mPos, 2, bounds)) {
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && !globalIsEditing) {
            isEditing = true;
            globalIsEditing = true;
        }
    } else {
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && isEditing) {
            isEditing = false;
            globalIsEditing = false;
        }
    }
}

void TextBox::UpdateTextBox() {
    if (!this->isEditing) {
        return;
    }

    const char pressedChar = static_cast<char>(GetCharPressed());
    if (pressedChar > 0) {
        buffer.push_back(pressedChar);
    }
    if (IsKeyPressed(KEY_BACKSPACE) && !buffer.empty()) {
        buffer.pop_back();
    }
    if (IsKeyPressedRepeat(KEY_BACKSPACE) && !buffer.empty()) {
        buffer.pop_back();
    }
}

void TextBox::Update(const Rectangle bounds, const Vector2 &mPos, bool &globalIsEditing) {
    this->UpdateEditMode(bounds, mPos, globalIsEditing);
    this->UpdateTextBox();
    this->DrawTextBox(bounds);
}
