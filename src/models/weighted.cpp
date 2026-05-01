#include "weighted.h"
#include "../osm/tags.h"
#include "spdlog/spdlog.h"

#include <cmath>
#include <cstdint>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <queue>

bool Weighted::FindPath(OSMGraph& graph, UserInterface& ui)
{
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
        for (std::pair<OSMNodeID, OSMWayID> neighbor : adj_list[current])
        {
            OSMNodeID neighborID = neighbor.first;
            OSMWayID edgeWay = neighbor.second;

            const OSMNode& nodeA = graph.GetNode(current);
            const OSMNode& nodeB = graph.GetNode(neighborID);
            const OSMWay& way = graph.GetWay(edgeWay);

            auto speedTag = way.tags.find("maxspeed");
            auto highwayTag = way.tags.find("highway");
            auto litTag = way.tags.find("lit");
            auto smoothnessTag = way.tags.find("smoothness");

            double speed = (speedTag != way.tags.end())
               ? ParseMaxSpeed(speedTag->second)
               : GetDefaultSpeed(highwayTag->second);

            double speedMS = KmhToMS(speed);
            double distance = Equirectangular(nodeA.location, nodeB.location);

            double lit = (litTag != way.tags.end() && litTag->second == "yes") ? 1.0 : 10.0;
            double smoothness = (smoothnessTag != way.tags.end() ? GetRoadSmoothness(smoothnessTag->second) : 5.0);

            double timeToDrive = distance / speedMS;

            std::pair<OSMNodeID, double> null = {0, 0.0};

            std::pair<OSMNodeID, double> nearestFuel = (ui.GetGasStation() != 0.0) ?    graph.GetNearestNode("amenity=fuel", nodeB.location) : null;
            std::pair<OSMNodeID, double> nearestCafe = (ui.GetCafe() != 0.0) ?          graph.GetNearestNode("amenity=cafe", nodeB.location) : null;
            std::pair<OSMNodeID, double> nearestTourism = (ui.GetTourism() != 0.0) ?    graph.GetNearestNode("tourism", nodeB.location) : null;

            double alt = cost[current] + (ui.GetDistance()      * distance) + 
                                         (ui.GetTime()          * timeToDrive) + 
                                         (ui.GetLitRoads()      * lit * distance) +
                                         (ui.GetSmoothness()    * smoothness * distance) +
                                         (ui.GetGasStation()    * pow(nearestFuel.second * distance, 2)) +
                                         (ui.GetCafe()          * pow(nearestCafe.second * distance, 2)) +
                                         (ui.GetTourism()       * pow(nearestTourism.second * distance, 2));

            //double alt = cost[current] + (nearestDist * distance);
            
            if (alt < cost[neighborID])
            {
                cost[neighborID] = alt;
                prev[neighborID] = current;

                // Push the neighbor onto the priority queue with cost
                p_queue.push({ alt, neighborID });
            }
        }
    }

    return false;
}