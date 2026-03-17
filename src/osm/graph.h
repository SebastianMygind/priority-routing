#pragma once
#include "raylib.h"
#include "raymath.h"

#include <vector>
#include <unordered_map>
#include <cstdint>
#include <string>
#include <set>
#include <utility>

typedef uint64_t OSMNodeID;
typedef uint64_t OSMWayID;

struct OSMNode 
{
    double lat, lon;
};

struct OSMWay 
{
    std::vector<OSMNodeID> nodes;
    std::unordered_map<std::string, std::string> tags;
};

typedef OSMNode Coord;

class OSMGraph
{
public:
    OSMGraph();

    bool load(const std::string& path);
    bool ParsePBF(const std::string& path);
    bool ParseXML(std::string path);
    bool BuildAdjList();

    inline OSMNode GetNode(OSMNodeID id) const { return nodes.at(id); }
    inline OSMWay  GetWay(OSMWayID id) const   { return ways.at(id); }

    OSMNodeID GetNodeA() const            { return selectedNodeA; }
    OSMNodeID GetNodeB() const            { return selectedNodeB; }
    auto      GetAdjList() const          { return adj_list;      }
    auto      GetAdjListDist() const      { return adj_list_dist; }
    auto      GetAdjListPrev() const      { return adj_list_prev; }
    void      SetNodeA(OSMNodeID id)      { selectedNodeA = id; }
    void      SetNodeB(OSMNodeID id)      { selectedNodeB = id; }
    void      ClearPath()                 { selectedPath.clear(); }
    void      InsertPath(OSMNodeID path)  { selectedPath.insert(path); }

    // Stats
    size_t GetNodeCount() const        { return nodes.size(); }
    size_t GetWayCount() const         { return ways.size(); }

private:
    std::unordered_map<OSMNodeID, OSMNode> nodes;
    std::unordered_map<OSMWayID,  OSMWay>  ways;

    std::set<OSMNodeID> selectedPath;
    OSMNodeID selectedNodeA;
    OSMNodeID selectedNodeB;

    std::unordered_map<uint64_t, std::vector<uint64_t>> adj_list;
    std::unordered_map<uint64_t, double>   adj_list_dist;
    std::unordered_map<uint64_t, uint64_t> adj_list_prev;

    friend class OSMRenderer;
    friend class OSMHandler;
};



Vector2 MercatorProjection(double lat, double lon);
Coord InverseMercatorProjection(float worldX, float worldY);
