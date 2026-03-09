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
    std::vector<uint64_t> nodes;
    std::unordered_map<std::string, std::string> tags;
};

typedef OSMNode Coord;

class OSMGraph
{
public:
    OSMGraph();

    bool ParseXML(std::string path);

    inline OSMNode  GetNode(uint64_t id) const  { return nodes.at(id); }
    inline OSMWay   GetWay(uint64_t id) const   { return ways.at(id); }
    inline uint64_t GetNodeA() const            { return selectedNodeA; }
    inline uint64_t GetNodeB() const            { return selectedNodeB; }
    inline void     SetNodeA(uint64_t id)       { selectedNodeA = id; }
    inline void     SetNodeB(uint64_t id)       { selectedNodeB = id; }
    inline void     ClearPath()                 { selectedPath.clear(); }

private:
    std::unordered_map<uint64_t, OSMNode> nodes;
    std::unordered_map<uint64_t, OSMWay>  ways;

    std::set<uint64_t> selectedPath;
    uint64_t selectedNodeA;
    uint64_t selectedNodeB;

    friend class OSMRenderer;
    friend class Dijkstra;
};



Vector2 MercatorProjection(double lat, double lon);
Coord InverseMercatorProjection(float worldX, float worldY);
