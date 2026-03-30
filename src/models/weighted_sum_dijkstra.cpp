#include "weighted_sum_dijkstra.h"

#include "dijkstra.h"
#include "../osm/tags.h"

#include <cmath>
#include <cstdint>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <queue>

bool WeightedDijkstra::FindPath(OSMGraph& graph)
{
    using PQNode = std::pair<double, uint64_t>;
    std::priority_queue<PQNode, std::vector<PQNode>, std::greater<>> p_queue;

    auto source = graph.GetNodeA();
    auto destination = graph.GetNodeB();

    auto adj_list = graph.GetAdjList();
    auto dist = graph.GetAdjListDist();
    auto prev = graph.GetAdjListPrev();

    dist[source] = 0;
    p_queue.push({dist[source], source});

    while (!p_queue.empty())
    {
        // Get the node with the smallest cost + heuristic
        uint64_t current = p_queue.top().second;
        p_queue.pop();

        // If we reached the end node, reconstruct the path
        if (current == destination)
        {
            while (current != 0xFFFFFFFF)
            {
                graph.InsertPath(current);
                current = prev[current];
            }
            return true;
        }

        // Update distances to neighbors
        for (std::pair<uint64_t, double> neighbor : adj_list[current])
        {
            OSMNodeID neighborID = neighbor.first;
            double neighborCost = neighbor.second;

            double alt = dist[current] + neighborCost;

            if (alt < dist[neighborID])
            {
                dist[neighborID] = alt;
                prev[neighborID] = current;

                // Push the neighbor onto the priority queue with cost
                p_queue.push({alt, neighborID });
            }
        }
    }

    return false;
}