#include "PathFinding.h"
#include <cmath>
#include <cstdint>
#include <vector>

bool Djikstra(Graph& graph, uint64_t start_node, uint64_t end_node, std::vector<uint64_t>& out_path)
{
    std::vector<double>   dist(graph.nodes.size(), INFINITY);
    std::vector<uint64_t> prev(graph.nodes.size(), 0xFFFFFFFF);
    std::vector<uint64_t> queue = {start_node};

    dist[start_node] = 0;

    std::vector< std::vector<uint64_t> > adj_list(graph.nodes.size());
    for (const Edge& edge : graph.edges) 
    {
        // Undirected graph, so add both directions (if we want directed, we'd only add one)
        for (uint64_t i = 0; i < edge.nodeRefs.size() - 1; i++)
        {
            adj_list[edge.nodeRefs[i]].push_back(edge.nodeRefs[i + 1]);
            adj_list[edge.nodeRefs[i + 1]].push_back(edge.nodeRefs[i]);
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