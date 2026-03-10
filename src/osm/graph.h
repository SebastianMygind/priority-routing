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

    bool ParseXML(std::string path);

    inline OSMNode   GetNode(OSMNodeID id) const { return nodes.at(id); }
    inline OSMWay    GetWay(OSMWayID id) const   { return ways.at(id); }
    inline OSMNodeID GetNodeA() const            { return selectedNodeA; }
    inline OSMNodeID GetNodeB() const            { return selectedNodeB; }
    inline void      SetNodeA(OSMNodeID id)      { selectedNodeA = id; }
    inline void      SetNodeB(OSMNodeID id)      { selectedNodeB = id; }
    inline void      ClearPath()                 { selectedPath.clear(); }

private:
    std::unordered_map<OSMNodeID, OSMNode> nodes;
    std::unordered_map<OSMWayID,  OSMWay>  ways;

    std::set<OSMNodeID> selectedPath;
    OSMNodeID selectedNodeA;
    OSMNodeID selectedNodeB;

    friend class OSMRenderer;
    friend class Dijkstra;
};



Vector2 MercatorProjection(double lat, double lon);
Coord InverseMercatorProjection(float worldX, float worldY);
