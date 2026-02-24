#include "Graph.h"
#include "raylib.h"
#include <algorithm>

Graph::Graph() : selected_node_a(UINT32_MAX), selected_node_b(UINT32_MAX)
{
    // Init default graph
    nodes = {
        {0.0F, 0.0F},     // 0
        {40.0F, 12.0F},    // 1
        {85.0F, 15.0F},    // 2
        {130.0F, 30.0F},   // 3
        {80.0F, 100.0F},    // 4
    };
    edges = {{{0, 1, 2, 3}}, {{2, 4}}};
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
    for (uint32_t i = 0; i < nodes.size(); i++) 
    {
        const Node& node = nodes[i];
        Color color = (i == selected_node_a || i == selected_node_b) ? SKYBLUE : MAROON;
        DrawCircle(static_cast<int>(node.x), static_cast<int>(node.y), 6.F, color);
    }
}