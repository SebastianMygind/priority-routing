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
#include <memory>
#include <cmath>

class Tree2D
{
public:
    Tree2D() {}

    void   BuildTree() { root_ = MakeTree(0, nodes_.size(), 0); }
    void   AddNode(const OSMNodeID nodeId, const Coord location) { nodes_.emplace_back(nodeId, location); }
    double Distance() const { return std::sqrt(best_dist_); }
    double DistanceSq() const { return best_dist_; }

    const OSMNodeID& Nearest(const Coord& pt);

private:
    struct node
    {
        OSMNodeID nodeId;
        Coord location;
        node* left_;
        node* right_;

        node(const OSMNodeID& nodeId, const Coord& pt) : nodeId(nodeId), location(pt), left_(nullptr), right_(nullptr) {}
    };

    node* root_ = nullptr;
    node* best_ = nullptr;
    double best_dist_ = 0;
    std::size_t visited_ = 0;
    std::vector<node> nodes_;

    node* MakeTree(size_t begin, size_t end, size_t index);
    void Nearest(node* root, const Coord& pt, std::size_t index);
};




class OSMGraph 
{
public:
    OSMGraph();

    bool load(const std::string& path);
    bool ParseOSMFile(const std::string& path);
    bool Build2DTree();
    bool BuildAdjList();
    
    OSMNodeID StringToNode(const std::string& input);

    inline OSMNode GetNode(OSMNodeID id) const { return nodes.at(id); }
    inline OSMWay GetWay(OSMWayID id) const { return ways.at(id); }

    inline auto& GetWays() const { return ways; }

    inline OSMNodeID GetNearestNode(const Coord& location) const { return tree2d->Nearest(location); }
    inline double GetNearestNodeDist() const { return tree2d->Distance(); }
    inline double GetNearestNodeDistSq() const { return tree2d->DistanceSq(); }

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

    std::string pathForOSM = "../data/copenhagen.osm.pbf";

    // Tags
    std::vector<OSMNodeID> nodesWithTourism;

private:
    std::unordered_map<OSMNodeID, OSMNode> nodes;
    std::unordered_map<OSMWayID, OSMWay> ways;

    std::vector<OSMNodeID> selectedPath;
    OSMNodeID selectedNodeA;
    OSMNodeID selectedNodeB;

    std::unordered_map<OSMNodeID, std::vector<std::pair<OSMNodeID, OSMWayID>>> adj_list;
    std::unordered_map<OSMNodeID, double> adj_list_dist;
    std::unordered_map<OSMNodeID, OSMNodeID> adj_list_prev;

    std::vector<OSMNodeID> getNodesWithTourism() const;
    file_format_t GetNodeAttributes();
    file_format_t GenerateNodeAttributes(attr_map_t attrInfo);

    // Nodes that is not a part of a way
    std::vector<OSMNodeID> places;

    std::unique_ptr<Tree2D> tree2d;
   
public:

    friend class OSMRenderer;
    friend class OSMHandler;
};

Vector2 MercatorProjection(Coord location);
Coord InverseMercatorProjection(Vector2 world);

double KmhToMS(double kmh);
double MphToMS(double mph);
double ParseMaxSpeed(const std::string& value);
double GetDefaultSpeed(const std::string& highway);
double Haversine(const Coord& a, const Coord& b);
double EquirectangularSq(const Coord& a, const Coord& b);
double EuclideanDistance(const Coord& a, const Coord& b);
double DistanceSq(const Coord& a, const Coord& b);


