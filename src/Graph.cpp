#include "Graph.h"
#include "raylib.h"

Graph::Graph() : selected_node_a(UINT32_MAX), selected_node_b(UINT32_MAX)
{
    // Init default graph
    nodes = {
        {0.0F, 10.0F},     // 0
        {40.0F, 12.0F},    // 1
        {85.0F, 15.0F},    // 2
        {130.0F, 18.0F},   // 3
        {180.0F, 22.0F},   // 4
        {230.0F, 30.0F},   // 5
        {120.0F, -30.0F},  // 6
        {88.0F, -10.0F},   // 7
        {83.0F, 60.0F},    // 8
        {80.0F, 110.0F},   // 9
        {20.0F, -50.0F},   // 10
        {70.0F, -60.0F},   // 11
        {120.0F, -55.0F},  // 12
        {170.0F, -45.0F},  // 13
        {155.0F, 55.0F},   // 14
        {170.0F, 95.0F},   // 15
        {-20.0F, 40.0F},   // 16
        {10.0F, 55.0F},    // 17
        {35.0F, 45.0F},    // 18
        {5.0F, 25.0F},     // 19
        {70.0F, -20.0F}    // 20
    };
    edges = {{0, 1},   {1, 2},   {2, 3},   {3, 4},   {4, 5},  {6, 7},   {7, 2},  {2, 8},
             {8, 9},   {10, 11}, {11, 12}, {12, 13}, {7, 20}, {20, 11}, {3, 14}, {14, 15},
             {16, 17}, {17, 18}, {18, 19}, {19, 16}, {19, 0}, {1, 20},  {20, 12}};
}

void Graph::DrawGraph() 
{
    for (const Edge& edge : edges) 
    {
        Node& node1 = nodes[edge.a];
        Node& node2 = nodes[edge.b];

        bool in_path =
            std::find(selected_path.begin(), selected_path.end(), edge.a) != selected_path.end() &&
            std::find(selected_path.begin(), selected_path.end(), edge.b) != selected_path.end();
        Color color = in_path ? SKYBLUE : GRAY;

        DrawLineEx({node1.x, node1.y}, {node2.x, node2.y}, 3, color);
    }

    // Draw vertices
    for (uint32_t i = 0; i < nodes.size(); i++) 
    {
        const Node& node = nodes[i];
        Color color = (i == selected_node_a || i == selected_node_b) ? SKYBLUE : MAROON;
        DrawCircle(static_cast<int>(node.x), static_cast<int>(node.y), 6.F, color);
    }
}