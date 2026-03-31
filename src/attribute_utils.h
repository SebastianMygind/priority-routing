#pragma once

#include <functional>
#include <map>

#include "osm_types.h"

enum class Goal : bool {
    Minimize,
    Maximize,
};

using attrFunc = std::function<void(
    const std::unordered_map<OSMNodeID, OSMNode>&,
    const std::vector<OSMNodeID>&,
    OSMNodeID,
    std::pair<OSMNodeID,
    double> &
)>;

using attrValue = std::tuple<attrFunc, Goal, std::vector<OSMNodeID>>;
using attr_map_t = std::map<std::string, attrValue>;

size_t GetAttrHash(attr_map_t map);
size_t DataSetHash(const std::string& filepath);
size_t CombineHash(size_t a, size_t b);

// This is the function that should be made for each attribute,
// where you change the values on the optimalPair on finding a better result.
// This is bruteforce, but again that is why we want to cache the results.
void GenericFunc(std::unordered_map<OSMNodeID, OSMNode>& nodes,
                 std::vector<OSMNodeID>& searchSpace,
                 OSMNodeID currentNode,
                 std::pair<OSMNodeID, double>& optimalPair);

void TourismFunc(const std::unordered_map<OSMNodeID, OSMNode> &nodes,
                 const std::vector<OSMNodeID> &searchSpace,
                 const OSMNodeID currentNode,
                 std::pair<OSMNodeID, double> &optimalPair);
