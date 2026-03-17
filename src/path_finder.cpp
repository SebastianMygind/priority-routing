#include "path_finder.h"
#include "models/dijkstra.h"
#include "models/a_star.h"
#include "spdlog/spdlog.h"
#include <chrono>
#include <memory>

const double EARTH_RADIUS = 6371000.0; // in meters

static double KmhToMS(double kmh)
{
    return kmh * 1000.0 / 3600.0;
}

static double MphToMS(double mph)
{
    return mph * 1609.34 / 3600.0;
}

std::optional<double> ParseMaxSpeed(const std::string& value)
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
    return KmhToMS(number);
}

static double DegToRad(double deg)
{
    return deg * PI / 180.0;
}

double Haversine(const OSMNode& a, const OSMNode& b)
{
    double lat1 = DegToRad(a.lat);
    double lat2 = DegToRad(b.lat);
    double dLat = lat2 - lat1;
    double dLon = DegToRad(b.lon - a.lon);

    double h = std::sin(dLat / 2) * std::sin(dLat / 2) +
               std::cos(lat1) * std::cos(lat2) *
               std::sin(dLon / 2) * std::sin(dLon / 2);

    double c = 2 * std::atan2(std::sqrt(h), std::sqrt(1 - h));

    return EARTH_RADIUS * c;
}

double EuclideanDistance(const OSMNode& a, const OSMNode& b) 
{
    double xDiff = (b.lon - a.lon) * cos((a.lat + b.lat) / 2);
    double yDiff = (b.lat - a.lat);

    return std::sqrt(pow(xDiff, 2) * pow(yDiff, 2));
}

void PathFinder(OSMGraph& graph, PathfindingModel model)
{
    std::unique_ptr<IPathFinder> pathfinder = nullptr;

    switch (model)
    {
        case PathfindingModel::Dijkstra:
            pathfinder = std::make_unique<Dijkstra>();
            break;
        case PathfindingModel::AStar:
            pathfinder = std::make_unique<AStar>();
            break;
        default:
            spdlog::error("Invalid pathfinding model selected");
            return;
    }

    auto time_start = std::chrono::high_resolution_clock::now();
    pathfinder->FindPath(graph);
    auto time_end = std::chrono::high_resolution_clock::now();

    std::chrono::duration<double, std::milli> duration = time_end - time_start;
    spdlog::info("Time taken for pathfinding: {} ms", duration.count());
    
};