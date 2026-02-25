#include "PathFinding.h"
#include <cmath>
#include <cstdint>
#include <vector>
#include <unordered_map>
#include <algorithm>

bool Djikstra(Graph& graph, uint64_t start_node, uint64_t end_node, std::vector<uint64_t>& out_path)
{
    std::unordered_map<uint64_t, double>   dist;
    std::unordered_map<uint64_t, uint64_t> prev;
    std::vector<uint64_t> queue = {start_node};

    for (auto& node : graph.nodes)
    {
        dist.insert({node.first, INFINITY});
        prev.insert({node.first, 0xFFFFFFFF});
    }

    dist[start_node] = 0;

    std::unordered_map< uint64_t, std::vector<uint64_t> > adj_list;
    for (const auto& edge : graph.edges)
    {
        const auto& nodes = edge.nodeRefs;

        for (size_t i = 0; i + 1 < nodes.size(); ++i)
        {
            uint64_t a = nodes[i];
            uint64_t b = nodes[i + 1];

            adj_list[a].push_back(b);
            adj_list[b].push_back(a);  // reverse direction
        }
    }

    while (!queue.empty()) 
    {
        // Find the node in the queue with the smallest distance
        uint64_t current = queue[0];
        for (uint64_t node : queue) 
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
        for (uint64_t neighbor : adj_list[current]) 
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