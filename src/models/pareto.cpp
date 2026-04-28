#include "pareto.h"
#include "../osm/tags.h"

#include <cmath>
#include <cstdint>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <queue>

using LabelPtr = std::shared_ptr<struct Label>;
using LabelSet = std::vector<LabelPtr>;

struct Cost
{
    double distance;
    double time;
};

struct Label
{
    OSMNodeID node;
    Cost      cost;
    LabelPtr  prev;
};

struct Compare
{
    bool operator()(const LabelPtr& a, const LabelPtr& b) {
        if (a->cost.distance != b->cost.distance)
            return a->cost.distance > b->cost.distance;
        return a->cost.time > b->cost.time;
    }
};

// Does cost A dominate cost B?
bool Dominates(const Cost& a, const Cost& b) 
{
    return (a.distance <= b.distance && a.time <= b.time &&
           (a.distance < b.distance || a.time < b.time));
}



bool Pareto::FindPath(OSMGraph& graph, UserInterface& ui)
{
    auto adj_list = graph.GetAdjList();
    auto start = graph.GetNodeA();
    auto goal = graph.GetNodeB();


    std::unordered_map<OSMNodeID, LabelSet> frontier;

    std::priority_queue<LabelPtr, std::vector<LabelPtr>, Compare> p_queue;


    LabelPtr startLabel = std::make_shared<Label>();
    startLabel->node = start;
    startLabel->cost = { 0, 0 };
    startLabel->prev = nullptr;

    frontier[start].push_back(startLabel);
    p_queue.push(startLabel);


    while (!p_queue.empty() && !threadKill.load())
    {
        LabelPtr current = p_queue.top();
        p_queue.pop();


 /*       if (current->node == goal)
        {
            while (current != nullptr)
            {
                graph.InsertPath(current->node);
                current = current->prev;
            }
            return true;
        }*/


        for (auto [neighborId, wayId] : adj_list.at(current->node))
        {
            const OSMNode& nodeA = graph.GetNode(current->node);
            const OSMNode& nodeB = graph.GetNode(neighborId);
            const OSMWay&  way = graph.GetWay(wayId);

            auto speedTag = way.tags.find("maxspeed");
            auto highwayTag = way.tags.find("highway");

            double speed = (speedTag != way.tags.end())
                ? ParseMaxSpeed(speedTag->second)
                : GetDefaultSpeed(highwayTag->second);

            double speedMS = KmhToMS(speed);
            double distance = Equirectangular(
                nodeA.location,
                nodeB.location
            );

            double timeToDrive = distance / speedMS;

            Cost newCost
            {
                current->cost.distance + distance,
                current->cost.time + timeToDrive
            };

            bool dominated = false;

            LabelSet& neighborLabels = frontier[neighborId];

            for (LabelPtr& oldLabel : neighborLabels)
            {
                if (Dominates(oldLabel->cost, newCost))
                {
                    dominated = true;
                    break;
                }
            }

            if (dominated)
                continue;

            neighborLabels.erase(
                std::remove_if(
                    neighborLabels.begin(),
                    neighborLabels.end(),
                    [&](const LabelPtr& l)
                    {
                        return Dominates(newCost, l->cost);
                    }),
                neighborLabels.end()
            );

            LabelPtr candidate = std::make_shared<Label>();
            candidate->node = neighborId;
            candidate->cost = newCost;
            candidate->prev = current;

            neighborLabels.push_back(candidate);

            p_queue.push(candidate);

            if (neighborId == goal)
            {
                while (current != nullptr)
                {
                    graph.InsertPath(current->node);
                    current = current->prev;
                }

                return true;
            }
        }
    }

    //if (!frontier[goal].empty())
    //{
    //    return true;
    //}

    return false;
}