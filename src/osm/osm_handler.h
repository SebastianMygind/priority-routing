#pragma once

#include "graph.h"
#include "osmium/handler.hpp"

class OSMHandler : public osmium::handler::Handler
{
private:
    OSMGraph& graph;

public:
    explicit OSMHandler(OSMGraph& graph);

    void node(const osmium::Node& node);
    void way(const osmium::Way& way);
};