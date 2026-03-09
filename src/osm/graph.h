#pragma once
#include "raylib.h"
#include "raymath.h"

#include <vector>
#include <unordered_map>
#include <cstdint>
#include <string>
#include <set>
#include <utility>

struct OSMNode 
{
    double lat, lon;
};

struct OSMWay 
{
    std::vector<uint64_t> nodeRefs;
    std::unordered_map<std::string, std::string> tags;
};

typedef OSMNode Coord;
typedef std::vector<Vector2> Polygon;


class OSMGraph
{
public:
    OSMGraph();

public:
    std::unordered_map<uint64_t, OSMNode> nodes;
    std::unordered_map<uint64_t, OSMWay>  ways;

    std::set<uint64_t> selected_path;
    uint64_t selected_node_a;
    uint64_t selected_node_b;

    friend class OSMRenderer;
};

bool ParseOSM(std::string path, OSMGraph& out_graph);

Vector2 MercatorProjection(double lat, double lon);
Coord InverseMercatorProjection(float worldX, float worldY);
