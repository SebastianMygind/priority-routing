#pragma once

#include <cstdint>
#include <unordered_map>
#include <string>
#include <vector>

using OSMNodeID = uint64_t;
using OSMWayID = uint64_t;
using OSMPath = std::vector<OSMNodeID>;

struct Coord {
    double lat, lon;
};

struct OSMNode {
    Coord location;
    std::unordered_map<std::string, std::string> tags;
};

struct OSMWay {
    std::vector<OSMNodeID> nodes;
    std::unordered_map<std::string, std::string> tags;
};