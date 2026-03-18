#pragma once

#include <string>

#include "raylib.h"

class TextBox {
    std::string buffer;
    bool isEditing = false;

    void DrawTextBox(Rectangle bounds) const;

    void UpdateTextBox();

    void UpdateEditMode(Rectangle bounds, const Vector2 &mPos, bool &globalIsEditing);

public:
    TextBox() {
        buffer.reserve(1024);
    };

    void Update(Rectangle bounds, const Vector2 &mPos, bool &globalIsEditing);
};
