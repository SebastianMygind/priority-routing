#pragma once
#include "graph.h"
#include "raylib.h"
#include "raymath.h"
#include "../Window.h"

#include <vector>
#include <unordered_map>
#include <memory>
#include <thread>


typedef std::vector<Vector2> Polygon;

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
    int layer;
};

using MapObjects = std::vector<MapObject>;
using LayeredMapObjects = std::vector<MapObjects>;

class QuadNode 
{
public:
    QuadNode(const AABB& boundary, int capacity)
        : boundary(boundary), capacity(capacity), divided(false) {}

    bool InsertNode(const MapObject& obj);
    bool InsertWay(const MapObject& obj);
    void Query(const AABB& range, LayeredMapObjects* foundNodes, LayeredMapObjects* foundWays, int depth) const;
    void QueryQuads(const AABB& range, std::vector<AABB>* foundBounds, int depth) const;

private:
    AABB boundary;
    size_t capacity;
    bool divided;

    std::vector<MapObject> nodes;
    std::vector<MapObject> ways;

    std::unique_ptr<QuadNode> nw;
    std::unique_ptr<QuadNode> ne;
    std::unique_ptr<QuadNode> sw;
    std::unique_ptr<QuadNode> se;

    void Subdivide();
};

// Packet structs
struct Road 
{
    Vector2 p1, p2;
    float   width;
    Color   color;
};
struct Poly 
{
    Polygon shape;
    Color   color;
};
struct Node 
{
    Vector2 center;
    float   radius;
    Color   color;
};
struct Quad 
{
    Rectangle rect;
    float   width;
    Color   color;
};
struct RenderPacket 
{
    std::vector<Poly> polys;
    std::vector<Road> roads;
    std::vector<Road> path;
    std::vector<Node> nodes;
    std::vector<Quad> quads;
};

class OSMRenderer
{
public:
    OSMRenderer(OSMGraph* graph);

    void BuildQuadTree();
    void PrepareGraph(Camera2D& camera, Window window, Vector2 mouseWorldPos);
    void UpdateGraph(Camera2D& camera, Window window, Vector2 mouseWorldPos);
    void DrawGraph();
    void FinishThread() { if (renderThread.joinable()) renderThread.join(); }

    void ToggleQuad() { showQuad = !showQuad; }

    // Rendering stats (call after DrawGraph)
    inline size_t GetNodeRenderCount() const 
    { 
        size_t a = 0; 
        for (const MapObjects& layer : m_NodesToRender) { a += layer.size(); }
        return a; 
    }
    inline size_t GetWayRenderCount() const 
    { 
        size_t a = 0; 
        for (const MapObjects& layer : m_WaysToRender) { a += layer.size(); }
        return a; 
    }

private:
    Polygon& CachePolygonSingle(OSMWayID wayId, const OSMWay& way);

    std::thread renderThread;
    std::atomic<bool> threadDone { false };
    RenderPacket bufferedPacket;
    RenderPacket nextPacket;

    bool showQuad = false;

public:
    OSMGraph* m_pGraph;
    QuadNode m_Tree;
    QuadNode m_Tree1;
    std::unordered_map<OSMWayID, Polygon> m_CachedPolygons; // WayID, Polygon data

    // Temporary buffers for rendering
    LayeredMapObjects m_NodesToRender;
    LayeredMapObjects m_WaysToRender;
};

AABB GetScreenLocationBounds(Camera2D camera, float w, float h);