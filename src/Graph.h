#pragma once
#include <vector>
#include <cstdint>

struct Node 
{
    float x, y;
};

struct Edge 
{
    std::vector<uint32_t> nodeRefs;
    //uint32_t a, b;
};


class Graph 
{
public:
    std::vector<Node> nodes;
    std::vector<Edge> edges;

    std::vector<uint32_t> selected_path;
    uint32_t selected_node_a;
    uint32_t selected_node_b;

    Graph();

    void DrawGraph();
};