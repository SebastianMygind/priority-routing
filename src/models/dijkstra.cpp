#include "dijkstra.h"
#include <cmath>
#include <cstdint>
#include <vector>
#include <unordered_map>
#include <algorithm>

bool Dijkstra::FindPath(
    Graph& graph,
    uint64_t start_node,
    uint64_t end_node,
    std::vector<uint64_t>& out_path)
{
    std::unordered_map<uint64_t, double>   dist;
    std::unordered_map<uint64_t, uint64_t> prev;
    std::vector<uint64_t> queue = {start_node};

    std::unordered_map< uint64_t, std::vector<uint64_t> > adj_list;
    for (const Way& way : graph.ways)
    {
        const std::vector<uint64_t>& nodes = way.nodeRefs;

        if (way.tags.find("highway") == way.tags.end())
            continue;

        auto tag = way.tags.find("oneway");
        const bool oneWay = (tag != way.tags.end()) && (tag->second == "yes");

        for (size_t i = 0; i + 1 < nodes.size(); ++i)
        {
            uint64_t a = nodes[i];
            uint64_t b = nodes[i + 1];

            dist.insert({a, INFINITY});
            dist.insert({b, INFINITY});
            prev.insert({a, 0xFFFFFFFF});
            prev.insert({b, 0xFFFFFFFF});

            if (oneWay)
            {
                adj_list[a].push_back(b);
            }
            else
            {
                adj_list[a].push_back(b);
                adj_list[b].push_back(a);
            }
        }
    }

    dist[start_node] = 0;

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
                out_path.insert(current);
                current = prev[current];
            }
            //std::reverse(out_path.begin(), out_path.end());
            return true;
        }

        // Remove current from queue
        queue.erase(std::remove(queue.begin(), queue.end(), current), queue.end());

        // Update distances to neighbors
        for (uint64_t neighbor : adj_list[current]) 
        {
            double alt = dist[current] + sqrt(pow(graph.nodes[current].lat - graph.nodes[neighbor].lat, 2) + pow(graph.nodes[current].lon - graph.nodes[neighbor].lon, 2));
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