#pragma once
#include <vector>
#include <unordered_map>
#include <cstdint>
#include <string>

#include "raylib.h"

struct Node 
{
    double lat, lon;
};

struct Way 
{
    std::vector<uint64_t> nodeRefs;
    std::unordered_map<std::string, std::string> tags;
};


class Graph 
{
public:
    std::unordered_map<uint64_t, Node> nodes;
    std::vector<Way> ways;

    std::vector<uint64_t> selected_path;
    uint64_t selected_node_a;
    uint64_t selected_node_b;

    Graph();

    void DrawGraph(Camera2D camera, float screenWidth, float screenHeight);
};


inline Vector2 MercatorProjection(double lat, double lon, float screenHeight, float screenWidth);