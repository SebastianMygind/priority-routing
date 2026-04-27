#include "osm_handler.h"
#include "osmium/osm.hpp"

OSMHandler::OSMHandler(OSMGraph& graph) : graph(graph)
{
}

void OSMHandler::node(const osmium::Node& node)
{
    if (!node.location())
        return;

    OSMNode osmNode;

    osmNode.location = { 
        node.location().lat(),
        node.location().lon()
    };

    for (const auto& tag : node.tags()) {
        osmNode.tags.emplace(tag.key(), tag.value());
    }

    graph.nodes.emplace(node.id(), std::move(osmNode));
}

void OSMHandler::way(const osmium::Way& way)
{
    OSMWay osm_way;

    for (const auto& tag : way.tags())
    {
        osm_way.tags.emplace(tag.key(), tag.value());
    }

    for (const auto& node_ref : way.nodes())
    {
        osm_way.nodes.push_back(node_ref.ref());
    }

    graph.ways.emplace(way.id(), std::move(osm_way));
}
