#include "dijkstra.h"
#include "../osm/tags.h"
#include "spdlog/spdlog.h"

#include <cmath>
#include <cstdint>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <queue>

bool Dijkstra::FindPath(OSMGraph& graph, ObjectiveList objectives)
{
    if (objectives.empty())
    {
        spdlog::error("One objective is required.");
        return false;
    }

    using PQNode = std::pair<double, OSMNodeID>;
    std::priority_queue<PQNode, std::vector<PQNode>, std::greater<>> p_queue;

    auto source = graph.GetNodeA();
    auto destination = graph.GetNodeB();
    
    auto adj_list = graph.GetAdjList();
    auto cost = graph.GetAdjListDist();
    auto prev = graph.GetAdjListPrev();

    cost[source] = 0;
    p_queue.push({cost[source], source});

    while (!p_queue.empty() && !threadKill.load())
    {
        // Get the node with the smallest cost + heuristic
        OSMNodeID current = p_queue.top().second;
        p_queue.pop();

        // If we reached the end node, reconstruct the path
        if (current == destination)
        {
            OSMPath path;
            while (current != 0xFFFFFFFF)
            {
                path.push_back(current);
                current = prev[current];
            }
            std::reverse(path.begin(), path.end());
            graph.InsertPath(path);
            return true;
        }

        // Update distances to neighbors
        for (std::pair<OSMNodeID, OSMWayID> neighbor : adj_list.at(current))
        {
            OSMNodeID neighborID = neighbor.first;
            OSMWayID edgeWay = neighbor.second;

            const OSMNode& nodeA = graph.GetNode(current);
            const OSMNode& nodeB = graph.GetNode(neighborID);
            const OSMWay& way = graph.GetWay(edgeWay);

            double distance = Haversine(nodeA.location, nodeB.location);

            double alt = cost[current] + distance;
            
            if (alt < cost[neighborID])
            {
                cost[neighborID] = alt;
                prev[neighborID] = current;

                // Push the neighbor onto the priority queue with cost
                p_queue.push({alt, neighborID });
            }
        }
    }

    return false;
}