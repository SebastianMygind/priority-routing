#pragma once

#include <string>
#include <utility>

class Window {
public:
    //Logical units
    int height{720}, width{1280};
    std::string title;

    bool showDebug = true;
    bool showDebugVisuals = true;

    explicit Window(std::string i_title) : title(std::move(i_title)) {
    }
};
