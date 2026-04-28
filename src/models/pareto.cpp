#include "pareto.h"
#include "../osm/tags.h"
#include "spdlog/spdlog.h"

#include <cmath>
#include <cstdint>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <queue>


struct Cost
{
    double distance;
    double time;
};

inline Cost operator+(const Cost& a, const Cost& b)
{
    return {
        a.distance + b.distance,
        a.time + b.time
    };
}

// a dominates b if a is better/equal in both,
// and strictly better in at least one.
bool Dominates(const Cost& a, const Cost& b)
{
    return (a.distance <= b.distance &&
            a.time <= b.time &&
           (a.distance < b.distance ||
            a.time < b.time));
}

struct Label
{
    OSMNodeID node;
    Cost cost;

    // priority queue ordering (lexicographic)
    bool operator>(const Label& other) const
    {
        if(cost.distance != other.cost.distance)
            return cost.distance > other.cost.distance;

        return cost.time > other.cost.time;
    }
};

// Final route result
struct ParetoRoute
{
    std::vector<OSMNodeID> path;
    double totalDistance;
    double totalTime;
};

bool Pareto::FindPath(OSMGraph& graph, UserInterface& ui)
{
    // Pareto frontier at each node
    std::unordered_map<OSMNodeID, std::vector<Cost>> frontier;

    std::priority_queue<Label, std::vector<Label>, std::greater<Label>> pq;

    OSMNodeID start = graph.GetNodeA();
    OSMNodeID end = graph.GetNodeB();

    auto adj_list = graph.GetAdjList();

    frontier[start].push_back({0.0,0.0});
    pq.push({start,{0.0,0.0}});

    while(!pq.empty())
    {
        Label currentLabel = pq.top();
        pq.pop();

        OSMNodeID current = currentLabel.node;
        Cost currentCost = currentLabel.cost;

        OSMNode d = graph.GetNode(current);

        if (current == end)
        {
            return true;
        }

        printf("%ld \n", current);

        // Expand neighbors
        for (std::pair<OSMNodeID, OSMWayID> neighbor : adj_list.at(current))
        {
            OSMNodeID neighborID = neighbor.first;
            OSMWayID edgeWay     = neighbor.second;

            const OSMNode& nodeA = graph.GetNode(current);
            const OSMNode& nodeB = graph.GetNode(neighborID);
            const OSMWay&  way   = graph.GetWay(edgeWay);

            auto highwayTag = way.tags.find("highway");
            auto speedTag   = way.tags.find("maxspeed");

            double speed =
                (speedTag != way.tags.end())
                    ? ParseMaxSpeed(speedTag->second)
                    : GetDefaultSpeed(highwayTag->second);

            double speedMS  = KmhToMS(speed);
            double distance = Haversine(nodeA.location, nodeB.location);
            double timeToDrive = distance / speedMS;

            Cost edgeCost {
                distance,
                timeToDrive
            };

            Cost newCost = currentCost + edgeCost;

            //-----------------------------------
            // Pareto dominance check
            //-----------------------------------

            auto& labels = frontier[neighborID];

            bool dominated = false;

            // If any existing label dominates new one -> skip
            for(const Cost& c : labels)
            {
                if(Dominates(c, newCost))
                {
                    dominated = true;
                    break;
                }
            }

            if(dominated)
                continue;

            // Remove labels dominated by new label
            labels.erase(
                std::remove_if(
                    labels.begin(),
                    labels.end(),
                    [&](const Cost& c)
                    {
                        return Dominates(
                            newCost,
                            c
                        );
                    }),
                labels.end()
            );

            // Add new non-dominated label
            labels.push_back(newCost);

            pq.push({neighborID, newCost});
        }
    }

    
    
    // std::vector<ParetoRoute> results;

    // for(auto solution : frontier[end])
    // {
    //     ParetoRoute route;

    //     route.totalDistance = solution.second.distance;
    //     route.totalTime = solution.second.time;

    //     std::vector<OSMNodeID> reversed;

    //     std::pair<OSMNodeID, Cost> node = solution;

    //     while(node.first != 0)
    //     {
    //         reversed.push_back(node.first);

    //         node = frontier[node.first];
    //     }

    //     route.path.assign(
    //         reversed.rbegin(),
    //         reversed.rend()
    //     );

    //     results.push_back(route);
    // }
    return false;
}