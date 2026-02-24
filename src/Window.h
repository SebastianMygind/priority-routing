#ifndef PRIORITY_ROUTING_WINDOW_H
#define PRIORITY_ROUTING_WINDOW_H
#include <string>
#include <utility>


class Window {
public:
    int height, width;
    std::string title;

    explicit Window(std::string i_title) {
        width = 1280;
        height = 720;
        title = std::move(i_title);
    }
};


#endif //PRIORITY_ROUTING_WINDOW_H