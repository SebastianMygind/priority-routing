#include "Graph.h"
#include "raylib.h"
#include <algorithm>

Graph::Graph() : selected_node_a(UINT32_MAX), selected_node_b(UINT32_MAX)
{
    // Init default graph
    nodes = {
        {0, {0.0F, 0.0F}},     // 0
        {1, {40.0F, 12.0F}},    // 1
        {2, {85.0F, 15.0F}},    // 2
        {3, {130.0F, 30.0F}},   // 3
        {4, {80.0F, 100.0F}},    // 4

        {5, {100.0F, 100.0F}},    // 5
        {6, {150.0F, 150.0F}},    // 5
        
    };
    edges = {{{0, 1, 2, 3}}, {{2, 4}}, {{5, 6}}};
}

void Graph::DrawGraph() 
{
    for (const Edge& edge : edges) 
    {
        for (uint32_t i = 0; i < edge.nodeRefs.size() - 1; i++)
        {
            Node& node1 = nodes[edge.nodeRefs[i]];
            Node& node2 = nodes[edge.nodeRefs[i + 1]];

            bool in_path =
                std::find(selected_path.begin(), selected_path.end(), edge.nodeRefs[i]) != selected_path.end() &&
                std::find(selected_path.begin(), selected_path.end(), edge.nodeRefs[i + 1]) != selected_path.end();

            DrawLineEx({node1.x, node1.y}, {node2.x, node2.y}, 3, in_path ? SKYBLUE : GRAY);
        }
    }

    // Draw vertices
    for (auto& node : nodes)
    {
        Color color = (node.first == selected_node_a || node.first == selected_node_b) ? SKYBLUE : MAROON;
        DrawCircle(static_cast<int>(node.second.x), static_cast<int>(node.second.y), 1.2F, color);
    }
}