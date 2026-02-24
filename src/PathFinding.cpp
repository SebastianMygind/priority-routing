#include "PathFinding.h"
#include <cmath>
#include <cstdint>
#include <vector>
#include <algorithm>

bool Djikstra(Graph& graph, uint32_t start_node, uint32_t end_node, std::vector<uint32_t>& out_path)
{
    std::vector<double>    dist(graph.nodes.size(), INFINITY);
    std::vector<uint32_t> prev(graph.nodes.size(), 0xFFFFFFFF);
    std::vector<uint32_t> queue = {start_node};

    dist[start_node] = 0;

    std::vector< std::vector<uint32_t> > adj_list(graph.nodes.size());
    for (const Edge& edge : graph.edges) 
    {
        // Undirected graph, so add both directions (if we want directed, we'd only add one)
        adj_list[edge.a].push_back(edge.b);
        adj_list[edge.b].push_back(edge.a);
    }

    while (!queue.empty()) 
    {
        // Find the node in the queue with the smallest distance
        uint32_t current = queue[0];
        for (uint32_t node : queue) 
        {
            if (dist[node] < dist[current]) {
                current = node;
            }
        }

        // If we reached the end node, reconstruct the path
        if (current == end_node) 
        {
            out_path.clear();
            while (current != 0xFFFFFFFF) 
            {
                out_path.push_back(current);
                current = prev[current];
            }
            std::reverse(out_path.begin(), out_path.end());
            return true;
        }

        // Remove current from queue
        queue.erase(std::remove(queue.begin(), queue.end(), current), queue.end());

        // Update distances to neighbors
        for (uint32_t neighbor : adj_list[current]) 
        {
            double alt = dist[current] + sqrt(pow(graph.nodes[current].x - graph.nodes[neighbor].x, 2) + pow(graph.nodes[current].y - graph.nodes[neighbor].y, 2));
            if (alt < dist[neighbor]) 
            {
                dist[neighbor] = alt;
                prev[neighbor] = current;
                if (std::find(queue.begin(), queue.end(), neighbor) == queue.end()) {
                    queue.push_back(neighbor);
                }
            }
        }
    }

    return false;
}