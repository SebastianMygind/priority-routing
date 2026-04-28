#pragma once

#include "graph.h"
#include "osmium/handler.hpp"

class OSMHandler : public osmium::handler::Handler
{
public:
    explicit OSMHandler(OSMGraph& graph);

    void node(const osmium::Node& node);
    void way(const osmium::Way& way);

private:
    OSMGraph& graph;

};