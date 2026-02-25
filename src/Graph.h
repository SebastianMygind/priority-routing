#pragma once
#include <vector>
#include <unordered_map>
#include <cstdint>

struct Node 
{
    float x, y;
};

struct Edge 
{
    std::vector<uint64_t> nodeRefs;
    std::unordered_map<std::string, std::string> tags;
};


class Graph 
{
public:
    std::unordered_map<uint64_t, Node> nodes;
    std::vector<Edge> edges;

    std::vector<uint64_t> selected_path;
    uint64_t selected_node_a;
    uint64_t selected_node_b;

    Graph();

    void DrawGraph();
};