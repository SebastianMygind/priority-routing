#include "a_star.h"
#include "../osm/tags.h"

#include <cmath>
#include <cstdint>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <queue>
#include <sstream>
#include <functional>

bool AStar::FindPath(OSMGraph& graph)
{
    std::unordered_map<uint64_t, double>   dist;
    std::unordered_map<uint64_t, uint64_t> prev;

    using PQNode = std::pair<double, uint64_t>;
    std::priority_queue<PQNode, std::vector<PQNode>, std::greater<>> p_queue;
    std::unordered_map< uint64_t, std::vector<uint64_t> > adj_list;

    for (const auto& wayPair : graph.ways)
    {
        const OSMWay& way = wayPair.second;
        
        const std::vector<uint64_t>& nodes = way.nodes;

        auto highway = way.tags.find("highway");
        if (highway == way.tags.end())
            continue;

        if (kDrivableHighways.find(highway->second) == kDrivableHighways.end())
            continue;

        auto oneWayTag = way.tags.find("oneway");
        const bool oneWay = (oneWayTag != way.tags.end()) && (oneWayTag->second == "yes");

        for (size_t i = 0; i + 1 < nodes.size(); ++i)
        {
            uint64_t a = nodes[i];
            uint64_t b = nodes[i + 1];

            dist.insert({a, INFINITY});
            dist.insert({b, INFINITY});
            prev.insert({a, 0xFFFFFFFF});
            prev.insert({b, 0xFFFFFFFF});

            adj_list[a].push_back(b);

            if (!oneWay)
            {
                adj_list[b].push_back(a);
            }
        }
    }

    dist[graph.GetNodeA()] = 0;
    p_queue.push({dist[graph.GetNodeA()], graph.GetNodeA()});

    while (!p_queue.empty())
    {
        // Get the node with the smallest cost + heuristic
        uint64_t current = p_queue.top().second;
        p_queue.pop();

        // If we reached the end node, reconstruct the path
        if (current == graph.GetNodeB()) 
        {
            graph.selectedPath.clear();
            while (current != 0xFFFFFFFF) 
            {
                graph.selectedPath.insert(current);
                current = prev[current];
            }
            return true;
        }

        // Update distances to neighbors
        for (uint64_t neighbor : adj_list[current]) 
        {
            double alt = dist[current] + Haversine(graph.nodes.at(current), graph.nodes.at(neighbor));
            
            if (alt < dist[neighbor]) 
            {
                dist[neighbor] = alt;
                prev[neighbor] = current;

                // Calculate the heuristic value
                double h = Haversine(graph.nodes.at(neighbor), graph.nodes.at(graph.GetNodeB()));

                // Push the neighbor onto the priority queue with cost + heuristic
                p_queue.push({alt + h, neighbor});
            }
        }
    }

    return false;
}