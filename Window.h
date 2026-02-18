//
// Created by seb on 18/02/2026.
//

#ifndef PRIORITY_ROUTING_WINDOW_H
#define PRIORITY_ROUTING_WINDOW_H
#include <string>
#include <utility>


class Window {
public:
    int height, width;
    std::string title;

    explicit Window(std::string i_title) {
        width = 1920;
        height = 1080;
        title = std::move(i_title);
    }
};


#endif //PRIORITY_ROUTING_WINDOW_H