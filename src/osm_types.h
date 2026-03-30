#pragma once

#include <cstdint>
#include <unordered_map>
#include <string>
#include <vector>

using OSMNodeID = uint64_t;
using OSMWayID = uint64_t;

struct OSMNode {
    double lat, lon;
    std::unordered_map<std::string, std::string> tags;
};

struct OSMWay {
    std::vector<OSMNodeID> nodes;
    std::unordered_map<std::string, std::string> tags;
};

using Coord = OSMNode;
