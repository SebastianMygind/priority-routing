#include "Graph.h"
#include "tags.h"
#include "earcut.hpp"

#include <algorithm>
#include <cmath>
#include <array>

#include "rlgl.h"
#include "spdlog/spdlog.h"

OSMGraph::OSMGraph() : selected_node_a(UINT32_MAX), selected_node_b(UINT32_MAX)
{
}

OSMRenderer::OSMRenderer(OSMGraph* graph) : tree({11.271273842, 55.1959946, 13.008736992, 56.13105965}, 8), graph(graph)
{
}


void OSMRenderer::BuildQuadTree()
{
    spdlog::info("Building QuadTree..");

    for (const auto& wayPair : graph->ways)
    {
        const uint64_t& id = wayPair.first;
        const OSMWay& way = wayPair.second;

        bool isHighway = way.tags.find("highway") != way.tags.end();

        AABB box;
        OSMNode& node = graph->nodes[way.nodeRefs[0]];
        box.minX = box.maxX = node.lon;
        box.minY = box.maxY = node.lat;
        for (const uint64_t& nodeRef : way.nodeRefs) 
        {
            OSMNode& node = graph->nodes[nodeRef];
            box.minX = std::min(box.minX, node.lon);
            box.maxX = std::max(box.maxX, node.lon);
            box.minY = std::min(box.minY, node.lat);
            box.maxY = std::max(box.maxY, node.lat);

            if (isHighway)
            {
                MapObject obj;
                obj.id = nodeRef;
                obj.bounds = {node.lon, node.lat, node.lon, node.lat};
                tree.insertnode(obj);
            }
        }

        MapObject wayobj;
        wayobj.id = id;
        wayobj.bounds = box;
        tree.insertway(wayobj);
    }
}

void OSMRenderer::DrawGraph(Camera2D camera, float screenWidth, float screenHeight)
{
    std::unordered_map<uint64_t, OSMNode>& nodes = graph->nodes;
    std::unordered_map<uint64_t, OSMWay>&  ways = graph->ways;
    std::set<uint64_t>&  selected_path = graph->selected_path;

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
    tree.query({minLon, minLat, maxLon, maxLat}, &nodesToRender, &waysToRender);

    for (const MapObject& wayObj : waysToRender) 
    {   
        const uint64_t& id = wayObj.id;
        const OSMWay& way = ways[id];

        OSMNode& firstNode = nodes[way.nodeRefs.front()];
        OSMNode& lastNode = nodes[way.nodeRefs.back()];

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

            for (Vector2& point : building)
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
            
            for (uint32_t i = 0; i < way.nodeRefs.size() - 1; i++)
            {
                OSMNode& node1 = nodes[way.nodeRefs[i]];
                OSMNode& node2 = nodes[way.nodeRefs[i + 1]];

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
                    selected_path.contains(way.nodeRefs[i]) && selected_path.contains(way.nodeRefs[i + 1]);

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

    // Draw node
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

        bool isSelected = (id == graph->selected_node_a || id == graph->selected_node_b);
        DrawCircleV(
            p1,
            isSelected ? std::fmax(2.5F * (1.0 / camera.zoom), 0.1F) : 0.1F, 
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

    for (int i = 0; i < way.nodeRefs.size() - 1; i++)
    {
        OSMNode& n1 = graph->nodes[way.nodeRefs[i]];
        Vector2 p1 = MercatorProjection(n1.lat, n1.lon);
        Point point = {p1.x, p1.y};
        polygon[0].push_back(point);
    }

    std::vector<uint16_t> indices = mapbox::earcut<uint16_t>(polygon);

    for (int i = 0; i < indices.size(); i++)
    {
        Point& v = polygon[0][indices[i]];
        shape.push_back({v[0], v[1]});
    }

    m_CachedPolygons.insert({wayId, shape});

    auto polygonIt2 = m_CachedPolygons.find(wayId); 

    return polygonIt2->second;

}



Vector2 MercatorProjection(double lat, double lon)
{
    double centerLatitude = 55.6539977;
    double centerLongitude = 12.5422305;

    //constexpr double _PI = 3.14159265358979323846;

    // Clamp latitude to avoid infinity
    lat       = std::clamp(lat,      -85.05112878, 85.05112878);
    centerLatitude = std::clamp(centerLatitude,-85.05112878, 85.05112878);

    // Convert to radians
    double latRad        = lat        * PI / 180.0;
    double lonRad        = lon       * PI / 180.0;
    double centerLatRad  = centerLatitude  * PI / 180.0;
    double centerLonRad  = centerLongitude * PI / 180.0;

    // Proper Mercator projection
    double x  = lonRad;
    double y  = std::log(std::tan(PI / 4.0 + latRad / 2.0));

    double cx = centerLonRad;
    double cy = std::log(std::tan(PI / 4.0 + centerLatRad / 2.0));

    return {
        static_cast<float>((x - cx) * 500000.0),
        static_cast<float>((cy - y) * 500000.0) // flip Y for screen coords
    };

}

Coord InverseMercatorProjection(float worldX, float worldY)
{
    double centerLatitude  = 55.6539977;
    double centerLongitude = 12.5422305;

    constexpr double scale = 500000.0;

    // Clamp center latitude (same as forward projection)
    centerLatitude = std::clamp(centerLatitude, -85.05112878, 85.05112878);

    double centerLatRad = centerLatitude  * PI / 180.0;
    double centerLonRad = centerLongitude * PI / 180.0;

    // Reconstruct Mercator center Y
    double cy = std::log(std::tan(PI / 4.0 + centerLatRad / 2.0));

    // --- Invert screen transform ---
    double x = (worldX / scale) + centerLonRad;
    double y = cy - (worldY / scale);

    // --- Invert Mercator ---
    double lonRad = x;
    double latRad = 2.0 * std::atan(std::exp(y)) - PI / 2.0;

    return {
        latRad * 180.0 / PI,
        lonRad * 180.0 / PI
    };
}


void QuadTree::subdivide() 
{
    double midX = (boundary.minX + boundary.maxX) / 2.0;
    double midY = (boundary.minY + boundary.maxY) / 2.0;

    AABB nw_box{boundary.minX, midY, midX, boundary.maxY};
    AABB ne_box{midX, midY, boundary.maxX, boundary.maxY};
    AABB sw_box{boundary.minX, boundary.minY, midX, midY};
    AABB se_box{midX, boundary.minY, boundary.maxX, midY};

    nw = std::make_unique<QuadTree>(nw_box, capacity);
    ne = std::make_unique<QuadTree>(ne_box, capacity);
    sw = std::make_unique<QuadTree>(sw_box, capacity);
    se = std::make_unique<QuadTree>(se_box, capacity);

    divided = true;
}

bool QuadTree::insertnode(const MapObject& obj) 
{
    if (!boundary.intersects(obj.bounds))
        return false;

    if (nodes.size() < capacity) {
        nodes.push_back(obj);
        return true;
    }

    if (!divided)
        subdivide();

    if (nw->insertnode(obj)) return true;
    if (ne->insertnode(obj)) return true;
    if (sw->insertnode(obj)) return true;
    if (se->insertnode(obj)) return true;

    return false;
}

bool QuadTree::insertway(const MapObject& obj) {

    if (!boundary.intersects(obj.bounds))
        return false;

    if (ways.size() < capacity) {
        ways.push_back(obj);
        return true;
    }

    if (!divided)
        subdivide();

    bool inNW = nw->boundary.contains(obj.bounds);
    bool inNE = ne->boundary.contains(obj.bounds);
    bool inSW = sw->boundary.contains(obj.bounds);
    bool inSE = se->boundary.contains(obj.bounds);

    int count = inNW + inNE + inSW + inSE;

    if (count == 1) {
        if (inNW) return nw->insertway(obj);
        if (inNE) return ne->insertway(obj);
        if (inSW) return sw->insertway(obj);
        if (inSE) return se->insertway(obj);
    }

    // overlaps multiple children → keep here
    ways.push_back(obj);
    return true;
}

void QuadTree::query(const AABB& range, std::vector<MapObject>* foundNodes, std::vector<MapObject>* foundWays) const 
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


    if (!divided)
        return;

    nw->query(range, foundNodes, foundWays);
    ne->query(range, foundNodes, foundWays);
    sw->query(range, foundNodes, foundWays);
    se->query(range, foundNodes, foundWays);
}
