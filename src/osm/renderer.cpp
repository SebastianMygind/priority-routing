#include "renderer.h"
#include "tags.h"
#include "earcut.hpp"

#include "rlgl.h"
#include "spdlog/spdlog.h"

#include <array>

OSMRenderer::OSMRenderer(OSMGraph* graph) : m_pGraph(graph), m_Tree({11.271273842, 55.1959946, 13.008736992, 56.13105965}, 8)
{
}

void OSMRenderer::BuildQuadTree()
{
    spdlog::info("Building QuadTree..");

    for (const auto& wayPair : m_pGraph->ways)
    {
        const uint64_t& id = wayPair.first;
        const OSMWay& way = wayPair.second;

        bool isHighway = way.tags.find("highway") != way.tags.end();

        AABB box;
        OSMNode& node = m_pGraph->nodes[way.nodes[0]];
        box.minX = box.maxX = node.lon;
        box.minY = box.maxY = node.lat;
        for (const uint64_t& nodeRef : way.nodes) 
        {
            OSMNode& node = m_pGraph->nodes[nodeRef];
            box.minX = std::min(box.minX, node.lon);
            box.maxX = std::max(box.maxX, node.lon);
            box.minY = std::min(box.minY, node.lat);
            box.maxY = std::max(box.maxY, node.lat);

            if (isHighway)
            {
                MapObject obj;
                obj.id = nodeRef;
                obj.bounds = {node.lon, node.lat, node.lon, node.lat};
                m_Tree.InsertNode(obj);
            }
        }

        MapObject wayobj;
        wayobj.id = id;
        wayobj.bounds = box;
        m_Tree.InsertWay(wayobj);
    }
}

void OSMRenderer::DrawGraph(Camera2D camera, float screenWidth, float screenHeight, Vector2 cursor)
{
    std::unordered_map<uint64_t, OSMNode>& nodes = m_pGraph->nodes;
    std::unordered_map<uint64_t, OSMWay>&  ways = m_pGraph->ways;
    std::set<uint64_t>&  selected_path = m_pGraph->selectedPath;

    Vector2 topLeft     = GetScreenToWorld2D({ 0,0 }, camera);
    Vector2 bottomRight = GetScreenToWorld2D({ screenWidth, screenHeight }, camera);

    Coord topLeftLatLon     = InverseMercatorProjection(topLeft.x, topLeft.y);
    Coord bottomRightLatLon = InverseMercatorProjection(bottomRight.x, bottomRight.y);

    double minLat = bottomRightLatLon.lat;
    double maxLat = topLeftLatLon.lat;
    double minLon = topLeftLatLon.lon;
    double maxLon = bottomRightLatLon.lon;

    std::vector<MapObject> nodesToRender;
    std::vector<MapObject> waysToRender;
    nodesToRender.reserve(4096);
    waysToRender.reserve(4096);
    m_Tree.Query({minLon, minLat, maxLon, maxLat}, &nodesToRender, &waysToRender, 4);

    for (const MapObject& wayObj : waysToRender) 
    {   
        const uint64_t& id = wayObj.id;
        const OSMWay& way = ways[id];

        const AABB& bounds = wayObj.bounds;

         if ((bounds.maxY < minLat) || (bounds.minY > maxLat) ||
             (bounds.maxX < minLon) || (bounds.minX > maxLon))
         {
             continue;
         }

        if (auto tag = way.tags.find("building"); tag != way.tags.end())
        {
            Polygon& building = CachePolygonSingle(id, way);

            rlDisableBackfaceCulling();
            rlBegin(RL_TRIANGLES);
            rlColor4ub(200, 200, 200, 255);

            for (const Vector2& point : building)
            {
                rlVertex2f(point.x, point.y);
            }

            rlEnd();
        }
        else if (auto tag = way.tags.find("highway"); tag != way.tags.end())
        {
            Color lineColor = { 0,0,0,255 };
			float width = 0.2F;

            if (kMotorways.find(tag->second) != kMotorways.end())
            {
                lineColor = { 233,144,160,255 };
                width = 0.4F;
            }
            else if (kPrimary.find(tag->second) != kPrimary.end())
            {
                lineColor = { 191, 117, 36,255 };
                width = 0.4F;
            }

            if (kDrivableHighways.find(tag->second) == kDrivableHighways.end())
            {
                if (camera.zoom < 7.F)
                    continue;
                lineColor = { 100,100,100,255 };
                width = 0.05F;
            }
            
            for (size_t i = 0; i < way.nodes.size() - 1; i++)
            {
                OSMNode& node1 = nodes[way.nodes[i]];
                OSMNode& node2 = nodes[way.nodes[i + 1]];

                if ((node1.lat < minLat && node2.lat < minLat) ||
                    (node1.lat > maxLat && node2.lat > maxLat) ||
                    (node1.lon < minLon && node2.lon < minLon) ||
                    (node1.lon > maxLon && node2.lon > maxLon))
                {
                    continue;
                }

                Vector2 p1 = MercatorProjection(node1.lat, node1.lon);
                Vector2 p2 = MercatorProjection(node2.lat, node2.lon);
                    
                bool inPath =
                    selected_path.contains(way.nodes[i]) && selected_path.contains(way.nodes[i + 1]);

                DrawLineEx(
                    p1, 
                    p2, 
                    inPath ? std::fmax(2.5F * (1.0 / camera.zoom), width) : width, 
                    inPath ? SKYBLUE : lineColor
                );
            }
        }
    }

    if (camera.zoom < 7.F)
        return;

    // Draw nodes
    for (MapObject& nodeObj : nodesToRender)
    {
        uint64_t id = nodeObj.id;
        OSMNode& node = nodes[id];

        const AABB& bounds = nodeObj.bounds;

         if ((bounds.maxY < minLat) || (bounds.minY > maxLat) ||
             (bounds.maxX < minLon) || (bounds.minX > maxLon))
         {
             continue;
         }

        Vector2 p1 = MercatorProjection(node.lat, node.lon);

        float distToCursor = Vector2Distance(p1, cursor);

        if (distToCursor > 30.0F)
            continue;

        bool isSelected = (id == m_pGraph->GetNodeA() || id == m_pGraph->GetNodeB());
        bool isHovering = distToCursor < 0.2F;


        DrawCircleV(
            p1,
            isHovering ? 0.5F * (1.F / camera.zoom) * 25.F : (2.F / (distToCursor + 6.F)) * (1.F / camera.zoom) * 25.F, 
            isSelected ? SKYBLUE : MAROON
        );
    }
}

Polygon& OSMRenderer::CachePolygonSingle(uint64_t wayId, const OSMWay& way)
{
    auto polygonIt = m_CachedPolygons.find(wayId);

    // If polygon is already cached, use it
    if (polygonIt != m_CachedPolygons.end())
        return polygonIt->second;

    // Otherwise, create a triangulated polygon
    using Point = std::array<float, 2>;

    Polygon shape;
    std::vector<std::vector<Point>> polygon(1);

    for (size_t i = 0; i < way.nodes.size() - 1; i++)
    {
        OSMNode& n1 = m_pGraph->nodes[way.nodes[i]];
        Vector2 p1 = MercatorProjection(n1.lat, n1.lon);
        Point point = {p1.x, p1.y};
        polygon[0].push_back(point);
    }

    std::vector<uint16_t> indices = mapbox::earcut<uint16_t>(polygon);

    for (size_t i = 0; i < indices.size(); i++)
    {
        Point& v = polygon[0][indices[i]];
        shape.push_back({v[0], v[1]});
    }

    m_CachedPolygons.insert({wayId, shape});

    auto polygonIt2 = m_CachedPolygons.find(wayId); 

    return polygonIt2->second;
}


void QuadNode::Subdivide() 
{
    double midX = (boundary.minX + boundary.maxX) / 2.0;
    double midY = (boundary.minY + boundary.maxY) / 2.0;

    AABB nw_box{boundary.minX, midY, midX, boundary.maxY};
    AABB ne_box{midX, midY, boundary.maxX, boundary.maxY};
    AABB sw_box{boundary.minX, boundary.minY, midX, midY};
    AABB se_box{midX, boundary.minY, boundary.maxX, midY};

    nw = std::make_unique<QuadNode>(nw_box, capacity);
    ne = std::make_unique<QuadNode>(ne_box, capacity);
    sw = std::make_unique<QuadNode>(sw_box, capacity);
    se = std::make_unique<QuadNode>(se_box, capacity);

    divided = true;
}

bool QuadNode::InsertNode(const MapObject& obj) 
{
    if (!boundary.intersects(obj.bounds))
        return false;

    if (nodes.size() < capacity) {
        nodes.push_back(obj);
        return true;
    }

    if (!divided)
        Subdivide();

    if (nw->InsertNode(obj)) return true;
    if (ne->InsertNode(obj)) return true;
    if (sw->InsertNode(obj)) return true;
    if (se->InsertNode(obj)) return true;

    return false;
}

bool QuadNode::InsertWay(const MapObject& obj) 
{
    if (!boundary.intersects(obj.bounds))
        return false;

    if (ways.size() < capacity) {
        ways.push_back(obj);
        return true;
    }

    if (!divided)
        Subdivide();

    bool inNW = nw->boundary.contains(obj.bounds);
    bool inNE = ne->boundary.contains(obj.bounds);
    bool inSW = sw->boundary.contains(obj.bounds);
    bool inSE = se->boundary.contains(obj.bounds);

    int count = inNW + inNE + inSW + inSE;

    if (count == 1) {
        if (inNW) return nw->InsertWay(obj);
        if (inNE) return ne->InsertWay(obj);
        if (inSW) return sw->InsertWay(obj);
        if (inSE) return se->InsertWay(obj);
    }

    ways.push_back(obj);
    return true;
}

void QuadNode::Query(const AABB& range, std::vector<MapObject>* foundNodes, std::vector<MapObject>* foundWays, int depth) const 
{
    if (!boundary.intersects(range))
        return;

    if (foundNodes)
    {
        for (const auto& obj : nodes) {
            if (range.intersects(obj.bounds))
                foundNodes->push_back(obj);
        }
    }

    
    if (foundWays)
    {
        for (const auto& obj : ways) {
            if (range.intersects(obj.bounds))
                foundWays->push_back(obj);
        }
    }


    if (!divided || depth == 0)
        return;

    nw->Query(range, foundNodes, foundWays, depth - 1);
    ne->Query(range, foundNodes, foundWays, depth - 1);
    sw->Query(range, foundNodes, foundWays, depth - 1);
    se->Query(range, foundNodes, foundWays, depth - 1);
}
