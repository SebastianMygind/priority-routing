#include "pareto.h"
#include "../osm/tags.h"
#include "spdlog/spdlog.h"

#include <cmath>
#include <cstdint>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <queue>

using LabelPtr = std::shared_ptr<struct Label>;
using LabelSet = std::vector<LabelPtr>;

//struct Cost
//{
//    double signals;
//    double time;
//};

using Cost = std::vector<double>;

struct Label
{
    OSMNodeID node;
    Cost      cost;
    LabelPtr  prev;
};

struct Compare
{
    bool operator()(const LabelPtr& a, const LabelPtr& b) {
        if (a->cost[0] != b->cost[0])
            return a->cost[0] > b->cost[0];
        return a->cost[1] > b->cost[1];
    }
};

// Does cost A dominate cost B?
//bool Dominates(const Cost& a, const Cost& b) 
//{
//    return (a.signals <= b.signals && a.time <= b.time &&
//           (a.signals < b.signals || a.time < b.time));
//}

bool Dominates(const Cost& a, const Cost& b)
{
    bool strictlyBetter = false;

    for (size_t i = 0; i < a.size(); ++i) {
        if (a[i] > b[i])
            return false;

        if (a[i] < b[i])
            strictlyBetter = true;
    }

    return strictlyBetter;
}

bool EqualCost(const Cost& a, const Cost& b)
{
    for (size_t i = 0; i < a.size(); ++i) {
        if (a[i] != b[i])
            return false;
    }
    return true;
}


bool Pareto::FindPath(OSMGraph& graph, ObjectiveList objectives)
{
    if (objectives.empty())
    {
        spdlog::error("At least one objective is required.");
        return false;
    }

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

        //printf("%u \n", current->node);

        if (std::find(frontier[current->node].begin(), frontier[current->node].end(), current) == frontier[current->node].end())
        {
            continue;
        }

        //if (frontier[goal].size() > 2)
        //    break;

        //if (current->node == goal)
        //{
        //    OSMPath path;
        //    while (current != nullptr)
        //    {
        //        path.push_back(current->node);
        //        current = current->prev;
        //    }
        //    std::reverse(path.begin(), path.end());
        //    graph.InsertPath(path);
        //    return true;
        //}


        for (auto [neighborId, wayId] : adj_list.at(current->node))
        {
            const OSMNode& nodeA = graph.GetNode(current->node);
            const OSMNode& nodeB = graph.GetNode(neighborId);
            const OSMWay&  way = graph.GetWay(wayId);

            double distance = Haversine(nodeA.location, nodeB.location);

            Cost newCost;
            for (size_t i = 0; i < objectives.size(); ++i)
            {
                double edgeCost = objectives[i].func(graph, nodeA, nodeB, way, distance);
                newCost.push_back(current->cost[i] + edgeCost);
            }

            bool dominated = false;

            LabelSet& neighborLabels = frontier[neighborId];

            for (auto& oldLabel : neighborLabels)
            {
                if (EqualCost(oldLabel->cost, newCost) ||
                    Dominates(oldLabel->cost, newCost))
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
        }
    }

    if (!frontier[goal].empty())
    {
        spdlog::info("Pareto found {} paths", frontier[goal].size());
        for (const LabelPtr& label : frontier[goal])
        {
            //spdlog::info("Signals {}, Travel Time {} ", label->cost[0], label->cost[1]);
            LabelPtr it = label;
            OSMPath path;
            while (it != nullptr)
            {
                path.push_back(it->node);
                it = it->prev;
            }
            std::reverse(path.begin(), path.end());
            graph.InsertPath(path);
        }

        return true;
    }

    return false;
}