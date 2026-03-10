#pragma once
#include "graph.h"
#include "raylib.h"
#include "raymath.h"

#include <vector>
#include <unordered_map>
#include <memory>

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
};


class QuadNode 
{
public:
    QuadNode(const AABB& boundary, int capacity)
        : boundary(boundary), capacity(capacity), divided(false) {}

    bool InsertNode(const MapObject& obj);
    bool InsertWay(const MapObject& obj);
    void Query(const AABB& range, std::vector<MapObject>* foundNodes, std::vector<MapObject>* foundWays, int depth) const;
    void QueryQuads(const AABB& range, std::vector<AABB>* foundBounds, int depth) const;

private:
    AABB boundary;
    size_t capacity;
    bool divided;

    std::vector<MapObject> nodes;
    std::vector<MapObject> ways;

    std::vector<MapObject> waysLod;

    std::unique_ptr<QuadNode> nw;
    std::unique_ptr<QuadNode> ne;
    std::unique_ptr<QuadNode> sw;
    std::unique_ptr<QuadNode> se;

    void Subdivide();
};

struct OSMRendererSettings
{
    float   screenWidth;
    float   screenHeight;
    bool    drawObjBounds;
    bool    drawQuadBounds;
    Vector2 cursorPos;
};

class OSMRenderer
{
public:
    OSMRenderer(OSMGraph* graph);

    void BuildQuadTree();
    void DrawGraph(Camera2D& camera, OSMRendererSettings& settings);

    // Rendering stats (call after DrawGraph)
    inline size_t GetNodeRenderCount() const { return m_NodesToRender.size(); }
    inline size_t GetWayRenderCount() const { return m_WaysToRender.size(); }

private:
    Polygon& CachePolygonSingle(OSMWayID wayId, const OSMWay& way);
    void     DrawBounds(const AABB& bounds, Camera2D camera);

public:
    OSMGraph* m_pGraph;
    QuadNode m_Tree;
    QuadNode m_Tree1;
    std::unordered_map<OSMWayID, Polygon> m_CachedPolygons; // WayID, Polygon data

    // Temporary buffers for rendering
    std::vector<MapObject> m_NodesToRender;
    std::vector<MapObject> m_WaysToRender;
};
