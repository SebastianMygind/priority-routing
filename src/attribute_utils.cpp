#include "attribute_utils.h"

#include "osm/graph.h"

#include <filesystem>
#include <ranges>

// Common way to combine two hashes, taken from boost StackOverflow
size_t CombineHash(size_t a, size_t b) {
    return a ^ (b + 0x9e3779b9 + (a << 6) + (a >> 2));
}

size_t GetAttrHash(attr_map_t map) {
    const auto hashableStr = map | std::views::keys | std::views::join |
                             std::ranges::to<std::string>();

    return std::hash<std::string>{}(hashableStr);
}

size_t DataSetHash(const std::string& filepath) {

    if (!std::filesystem::exists(filepath)) {
        return 0;
    }

    const auto fileTime = std::filesystem::last_write_time(filepath);
    const auto fileSize = std::filesystem::file_size(filepath);
    const auto timeCount = fileTime.time_since_epoch().count();

    size_t hashSize = std::hash<size_t>{}(fileSize);
    size_t hashTime = std::hash<size_t>{}(timeCount);

    return CombineHash(hashTime, hashSize);
}

void TourismFunc(const std::unordered_map<OSMNodeID, OSMNode> &nodes,
                 const std::vector<OSMNodeID> &searchSpace,
                 const OSMNodeID currentNode,
                 std::pair<OSMNodeID, double> &optimalPair) {
    for (OSMNodeID nodeID : searchSpace) {
        const auto distance = Haversine(nodes.at(currentNode).location, nodes.at(nodeID).location);

        if (distance < optimalPair.second) {
            optimalPair = {nodeID, distance};
        }
    }
}
