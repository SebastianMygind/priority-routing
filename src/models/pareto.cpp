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

bool Dominates(const Cost& a, const Cost& b)
{
    return (
        a.distance <= b.distance &&
        a.time <= b.time &&
        (
            a.distance < b.distance ||
            a.time < b.time
        )
    );
}

// -------------------------------------
// Label stores path via parent pointer
// -------------------------------------

struct Label
{
    OSMNodeID node;
    Cost cost;

    std::shared_ptr<Label> parent;

    // PQ ordering
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
    using LabelPtr = std::shared_ptr<Label>;

    OSMNodeID start = graph.GetNodeA();
    OSMNodeID goal = graph.GetNodeB();

    auto adj_list = graph.GetAdjList();

    // Pareto labels per node
    std::unordered_map<OSMNodeID, std::vector<LabelPtr>> frontier;

    auto cmp =
        [](const LabelPtr& a, const LabelPtr& b)
        {
            return *a > *b;
        };

    std::priority_queue<LabelPtr, std::vector<LabelPtr>, decltype(cmp)> pq(cmp);

    auto startLabel = std::make_shared<Label>();

    startLabel->node = start;
    startLabel->cost = {0,0};
    startLabel->parent = nullptr;

    frontier[start].push_back(startLabel);

    pq.push(startLabel);

    while(!pq.empty() && !threadKill.load())
    {
        auto current = pq.top();
        pq.pop();

        OSMNodeID currentNode = current->node;

        if (currentNode == goal)
            break;
        printf("%u\n", currentNode);
        for(auto neighbor : adj_list.at(currentNode))
        {
            OSMNodeID neighborID = neighbor.first;
            OSMWayID edgeWay = neighbor.second;

            graph.nodeTest.push_back(neighborID);

            if (neighborID == goal)
                spdlog::info("goal vistited");

            const OSMNode& nodeA = graph.GetNode(currentNode);
            const OSMNode& nodeB = graph.GetNode(neighborID);
            const OSMWay& way = graph.GetWay(edgeWay);

            auto highwayTag = way.tags.find("highway");
            auto speedTag = way.tags.find("maxspeed");

            double speed = (speedTag != way.tags.end())
                ? ParseMaxSpeed(speedTag->second)
                : GetDefaultSpeed(highwayTag->second);

            double speedMS = KmhToMS(speed);
            double distance = Haversine(nodeA.location, nodeB.location);
            double driveTime = distance / speedMS;

            Cost edgeCost = { distance, driveTime };
            Cost newCost = current->cost + edgeCost;

            auto& labels = frontier[neighborID];
            bool dominated = false;

            for(auto& existing : labels)
            {
                if(Dominates(existing->cost, newCost))
                {
                    dominated = true;
                    break;
                }
            }

            if(dominated)
                continue;

            // Remove labels dominated
            labels.erase(
                std::remove_if(
                    labels.begin(),
                    labels.end(),
                    [&](const LabelPtr& l){
                        return Dominates(newCost, l->cost);
                    }),
                labels.end()
            );

            auto newLabel = std::make_shared<Label>();
            newLabel->node   = neighborID;
            newLabel->cost   = newCost;
            newLabel->parent = current;

            labels.push_back(newLabel);

            pq.push(newLabel);
        }
    }



    // std::vector<ParetoRoute> results;

    // for(auto& solution : frontier[goal])
    // {
    //     ParetoRoute route;
    //     route.totalDistance = solution->cost.distance;
    //     route.totalTime = solution->cost.time;

    //     std::vector<OSMNodeID> reversed;

    //     LabelPtr p = solution;

    //     while(p)
    //     {
    //         reversed.push_back(
    //             p->node
    //         );

    //         graph.InsertPath(p->node);

    //         p = p->parent;
    //     }

    //     route.path.assign(
    //         reversed.rbegin(),
    //         reversed.rend()
    //     );

    //     results.push_back(route);
    // }

    return false;
}