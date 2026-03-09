#include "dijkstra.h"
#include "../Tags.h"

#include <cmath>
#include <cstdint>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <queue>
#include <sstream>

static double DegToRad(double deg)
{
    return deg * M_PI / 180.0;
}

static double Haversine(const Node& a, const Node& b)
{
    constexpr double R = 6371000.0; // Earth radius in meters

    double lat1 = DegToRad(a.lat);
    double lat2 = DegToRad(b.lat);
    double dLat = lat2 - lat1;
    double dLon = DegToRad(b.lon - a.lon);

    double h = std::sin(dLat / 2) * std::sin(dLat / 2) +
               std::cos(lat1) * std::cos(lat2) *
               std::sin(dLon / 2) * std::sin(dLon / 2);

    double c = 2 * std::atan2(std::sqrt(h), std::sqrt(1 - h));
    return R * c;
}

static double KmHToMS(double kmh)
{
    return kmh * 1000.0 / 3600.0;
}

static double MphToMS(double mph)
{
    return mph * 1609.34 / 3600.0;
}

static std::optional<double> ParseMaxSpeed(const std::string& value)
{
    if (value.empty())
        return std::nullopt;

    std::stringstream ss(value);
    double number;
    ss >> number;

    if (ss.fail())
        return std::nullopt;

    if (value.find("mph") != std::string::npos)
        return MphToMS(number);

    // Default assume km/h
    return KmHToMS(number);
}

using AdjList = std::unordered_map<uint64_t, std::vector<std::pair<uint64_t, double>>>;


bool Dijkstra::FindPath(
    Graph& graph,
    uint64_t start_node,
    uint64_t end_node,
    std::vector<uint64_t>& out_path)
{
    std::unordered_map<uint64_t, double>   dist;
    std::unordered_map<uint64_t, uint64_t> prev;
    std::vector<uint64_t> queue = {start_node};

    // using PQNode = std::pair<double, uint64_t>;
    // std::priority_queue<PQNode, std::vector<PQNode>, std::greater<>> pq;

    std::unordered_map< uint64_t, std::vector<uint64_t> > adj_list;
    for (const auto& it : graph.ways)
    {
        const Way& way = it.second;
        
        const std::vector<uint64_t>& nodes = way.nodeRefs;

        auto highway = way.tags.find("highway");
        if (highway == way.tags.end())
            continue;

        if (kDrivableHighways.find(highway->second) == kDrivableHighways.end())
            continue;

        auto oneWayTag = way.tags.find("oneway");
        const bool oneWay = (oneWayTag != way.tags.end()) && (oneWayTag->second == "yes");

        // double speedMS = 0.0;

        // auto speedIt = way.tags.find("maxspeed");
        // if (speedIt != way.tags.end())
        // {
        //     auto parsed = ParseMaxSpeed(speedIt->second);
        //     if (parsed.has_value())
        //         speedMS = parsed.value();
        // }

        // double distMeters = Haversine(..);
        // double timeSeconds = distMeters / speedMS;

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

            double alt = dist[current] + Haversine(graph.nodes.at(current), graph.nodes.at(neighbor));
            
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