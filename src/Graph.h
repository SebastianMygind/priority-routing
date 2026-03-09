#pragma once
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <cstdint>
#include <string>
#include <set>
#include <utility>

#include "raylib.h"
#include "raymath.h"

// Route
struct Node 
{
    double lat, lon;
};

struct Way 
{
    std::vector<uint64_t> nodeRefs;
    std::unordered_map<std::string, std::string> tags;
};


typedef std::vector<Vector2> Polygon;

class Graph 
{
public:
    std::unordered_map<uint64_t, Node>    nodes;
    std::vector<std::pair<uint64_t, Way>> ways;

    // Filtered lists
    std::vector<std::pair<uint64_t, Node*>> nodes_highways;

    // Cache objects
    std::unordered_map<uint64_t, Polygon> m_CachedPolygons; // WayID, Polygon data

    std::set<uint64_t> selected_path;
    uint64_t selected_node_a;
    uint64_t selected_node_b;

    Graph();

    void DrawGraph(Camera2D camera, float screenWidth, float screenHeight);

    Polygon& CachePolygonSingle(uint64_t wayId, const Way& way);
};


inline Vector2 MercatorProjection(double lat, double lon);

inline Node InverseMercatorProjection(float screenX, float screenY);