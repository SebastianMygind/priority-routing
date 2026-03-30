#pragma once
#include "raylib.h"
#include "../osm_types.h"
#include "../attribute_utils.h"
#include "../caching.h"

#include <vector>
#include <unordered_map>
#include <cstdint>
#include <string>
#include <set>
#include <utility>

class OSMGraph {
public:
    OSMGraph();

    bool load(const std::string& path);
    bool ParsePBF(const std::string& path);
    bool ParseXML(std::string path);
    bool BuildAdjList();

    inline OSMNode GetNode(OSMNodeID id) const { return nodes.at(id); }
    inline OSMWay GetWay(OSMWayID id) const { return ways.at(id); }

    OSMNodeID GetNodeA() const { return selectedNodeA; }
    OSMNodeID GetNodeB() const { return selectedNodeB; }
    auto GetAdjList() const { return adj_list; }
    auto GetAdjListDist() const { return adj_list_dist; }
    auto GetAdjListPrev() const { return adj_list_prev; }
    void SetNodeA(OSMNodeID id) { selectedNodeA = id; }
    void SetNodeB(OSMNodeID id) { selectedNodeB = id; }
    void ClearPath() { selectedPath.clear(); }
    void InsertPath(OSMNodeID path) { selectedPath.push_back(path); }

    // Stats
    size_t GetNodeCount() const { return nodes.size(); }
    size_t GetWayCount() const { return ways.size(); }

    // Tags
    std::vector<OSMNodeID> nodesWithTourism;
private:
    std::unordered_map<OSMNodeID, OSMNode> nodes;
    std::unordered_map<OSMWayID, OSMWay> ways;

    std::vector<OSMNodeID> selectedPath;
    OSMNodeID selectedNodeA;
    OSMNodeID selectedNodeB;

    std::unordered_map<uint64_t, std::vector<std::pair<uint64_t, double>>>
        adj_list;
    std::unordered_map<uint64_t, double> adj_list_dist;
    std::unordered_map<uint64_t, uint64_t> adj_list_prev;

    std::vector<OSMNodeID> getNodesWithTourism() const;
    std::pair<OSMNodeID, double> getNearestNode(
        const std::vector<OSMNodeID>& nodeGroup, OSMNodeID center) const;
    file_format_t GetNodeAttributes();
    file_format_t GenerateNodeAttributes(attr_map_t attrInfo);

    friend class OSMRenderer;
    friend class OSMHandler;
};

Vector2 MercatorProjection(double lat, double lon);
Coord InverseMercatorProjection(float worldX, float worldY);

double ParseMaxSpeed(const std::string& value);
double GetDefaultSpeed(const std::string& highway);
double Haversine(const OSMNode& a, const OSMNode& b);
double EuclideanDistance(const OSMNode& a, const OSMNode& b);
