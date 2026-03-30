#include "attribute_utils.h"

#include "osm/graph.h"

#include <ranges>

size_t GetAttrHash(attr_map_t map) {
    const auto hashableStr = map | std::views::keys | std::views::join |
                             std::ranges::to<std::string>();

    return std::hash<std::string>{}(hashableStr);
}

void TourismFunc(const std::unordered_map<OSMNodeID, OSMNode> &nodes,
                 const std::vector<OSMNodeID> &searchSpace,
                 const OSMNodeID currentNode,
                 std::pair<OSMNodeID, double> &optimalPair) {
    for (OSMNodeID nodeID : searchSpace) {
        const auto distance = Haversine(nodes.at(currentNode), nodes.at(nodeID));

        if (distance < optimalPair.second) {
            optimalPair = {nodeID, distance};
        }
    }
}
