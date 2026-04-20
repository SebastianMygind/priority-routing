#pragma once

#include <string>
#include <utility>
#include "raylib.h"

class Window 
{
public:
    //Logical units
    int height{720}, width{1280};
    std::string title;
    Vector2 dpi {1.0f, 1.0f};

    explicit Window(std::string i_title) : title(std::move(i_title)) {
    }
};
