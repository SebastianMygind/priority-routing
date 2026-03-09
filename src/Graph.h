#pragma once
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <cstdint>
#include <string>
#include <set>
#include <utility>
#include <memory>

#include "raylib.h"
#include "raymath.h"


// Quad-tree bounding box
struct AABB 
{
    double minX, minY;
    double maxX, maxY;

    bool contains(double x, double y) const {
        return x >= minX && x <= maxX &&
               y >= minY && y <= maxY;
    }

    bool contains(const AABB& other) const {
    return other.minX >= minX &&
           other.maxX <= maxX &&
           other.minY >= minY &&
           other.maxY <= maxY;
    }

    bool intersects(const AABB& other) const {
        return !(other.minX > maxX || 
                 other.maxX < minX ||
                 other.minY > maxY ||
                 other.maxY < minY);
    }
};

struct MapObject 
{
    uint64_t id;
    AABB bounds;
};

class QuadTree 
{
public:
    QuadTree(const AABB& boundary, int capacity)
        : boundary(boundary), capacity(capacity), divided(false) {}

    bool insertnode(const MapObject& obj);
    bool insertway(const MapObject& obj);
    void query(const AABB& range, std::vector<MapObject>* foundNodes, std::vector<MapObject>* foundWays) const;

private:
    AABB boundary;
    int capacity;
    bool divided;

    std::vector<MapObject> nodes;
    std::vector<MapObject> ways;

    std::unique_ptr<QuadTree> nw;
    std::unique_ptr<QuadTree> ne;
    std::unique_ptr<QuadTree> sw;
    std::unique_ptr<QuadTree> se;

    void subdivide();
};



// Route
struct OSMNode 
{
    double lat, lon;
};

struct OSMWay 
{
    std::vector<uint64_t> nodeRefs;
    std::unordered_map<std::string, std::string> tags;
};

typedef OSMNode Coord;
typedef std::vector<Vector2> Polygon;


class OSMGraph
{
public:
    OSMGraph();

public:
    std::unordered_map<uint64_t, OSMNode> nodes;
    std::unordered_map<uint64_t, OSMWay>  ways;

    std::set<uint64_t> selected_path;
    uint64_t selected_node_a;
    uint64_t selected_node_b;

    friend class OSMRenderer;
};


class OSMRenderer
{
public:
    OSMRenderer(OSMGraph* graph);

    void BuildQuadTree();
    void DrawGraph(Camera2D camera, float screenWidth, float screenHeight);

private:
    Polygon& CachePolygonSingle(uint64_t wayId, const OSMWay& way);

public:
    OSMGraph* graph;
    QuadTree tree;
    std::unordered_map<uint64_t, Polygon> m_CachedPolygons; // WayID, Polygon data
};


inline Vector2 MercatorProjection(double lat, double lon);
inline Coord InverseMercatorProjection(float worldX, float worldY);
