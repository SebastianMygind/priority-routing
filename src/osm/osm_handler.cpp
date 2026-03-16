#include "osm_handler.h"
#include "osmium/handler.hpp"
#include "osmium/osm.hpp"

OSMHandler::OSMHandler(OSMGraph& graph) : graph(graph)
{
}

void OSMHandler::node(const osmium::Node& node)
{
    if (!node.location())
        return;

    graph.nodes.emplace(node.id(), OSMNode{.lat=node.location().lat(), .lon=node.location().lon()});
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
