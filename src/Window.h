#pragma once

#include <string>
#include <utility>

class Window {
public:
    int height{720}, width{1280};
    std::string title;

    explicit Window(std::string i_title) : title(std::move(i_title)){}
};
