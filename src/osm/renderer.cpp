#include "renderer.h"
#include "tags.h"
#include "earcut.hpp"

#include "rlgl.h"
#include "spdlog/spdlog.h"

#include <array>

OSMRenderer::OSMRenderer(OSMGraph* graph) : m_pGraph(graph), m_Tree({ 4.4, 53.3, 16.2, 58.7 }, 8),
m_Tree1({ 4.4, 53.3, 16.2, 58.7 }, 8)
{
    m_WaysToRender.resize(2); // We have two layers: 0 for landuse, 1 for highways
}

void OSMRenderer::BuildQuadTree()
{
    spdlog::info("Building QuadTree..");

    for (const auto& wayPair : m_pGraph->ways)
    {
        const uint64_t& id = wayPair.first;
        const OSMWay& way = wayPair.second;

        auto highwayTag = way.tags.find("highway");
        auto landuseTag = way.tags.find("landuse");
        bool isHighway = highwayTag != way.tags.end();

        bool isZoomLevel = isHighway && (kMotorways.find(highwayTag->second) != kMotorways.end() ||
                                         kPrimary.find(highwayTag->second) != kPrimary.end() ||
                                         kSecondary.find(highwayTag->second) != kSecondary.end() ||
                                         kTertiary.find(highwayTag->second) != kTertiary.end()) || 
                                         landuseTag != way.tags.end();
 

        AABB box;
        OSMNode& node = m_pGraph->nodes[way.nodes[0]];
        Coord& location = node.location;
        box.minX = box.maxX = location.lon;
        box.minY = box.maxY = location.lat;
        for (const uint64_t& nodeRef : way.nodes) 
        {
            OSMNode& node = m_pGraph->nodes[nodeRef];
            Coord& location = node.location;
            box.minX = std::min(box.minX, location.lon);
            box.maxX = std::max(box.maxX, location.lon);
            box.minY = std::min(box.minY, location.lat);
            box.maxY = std::max(box.maxY, location.lat);

            if (isHighway)
            {
                MapObject obj;
                obj.id = nodeRef;
                obj.bounds = { location.lon, location.lat, location.lon, location.lat };
                obj.layer = -1;
                m_Tree.InsertNode(obj);
            }
        }

        MapObject wayobj;
        wayobj.id = id;
        wayobj.bounds = box;
        wayobj.layer = (landuseTag != way.tags.end()) ? 0 : 1;
        m_Tree.InsertWay(wayobj);

        if (isZoomLevel)
            m_Tree1.InsertWay(wayobj);

    }
}

void OSMRenderer::DrawGraph(Camera2D& camera, OSMRendererSettings& settings)
{
    std::unordered_map<OSMNodeID, OSMNode>& nodes = m_pGraph->nodes;
    std::unordered_map<OSMWayID, OSMWay>&  ways = m_pGraph->ways;
    std::vector<OSMNodeID>&  selected_path = m_pGraph->selectedPath;

    AABB screenBounds = GetScreenLocationBounds(camera, settings.screenWidth, settings.screenHeight);

    m_NodesToRender.clear();
    m_WaysToRender[0].clear();
    m_WaysToRender[1].clear();

    if (camera.zoom > 4.F)
        m_Tree.Query(screenBounds, &m_NodesToRender, &m_WaysToRender, 20);
    else
        m_Tree1.Query(screenBounds, &m_NodesToRender, &m_WaysToRender, 20);

    for (const MapObjects& layer : m_WaysToRender)
    {
        for (const MapObject& wayObj : layer)
        {
            const OSMWayID& id = wayObj.id;
            const OSMWay& way = ways[id];

            const AABB& bounds = wayObj.bounds;

            if ((bounds.maxY < screenBounds.minY) || (bounds.minY > screenBounds.maxY) ||
                (bounds.maxX < screenBounds.minX) || (bounds.minX > screenBounds.maxX))
            {
                continue;
            }

            if (settings.drawObjBounds)
                DrawBounds(bounds, camera);

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
            else if (auto tag = way.tags.find("landuse"); tag != way.tags.end())
            {
                Polygon& landuse = CachePolygonSingle(id, way);

                rlDisableBackfaceCulling();
                rlBegin(RL_TRIANGLES);

                if (tag->second == "residential")
                    rlColor4ub(230, 230, 230, 235);
                else if (tag->second == "forest")
                    rlColor4ub(201, 207, 167, 255);
                else if (tag->second == "cemetery")
                    rlColor4ub(201, 207, 167, 255);
                else
                    rlColor4ub(240, 240, 240, 255);

                for (const Vector2& point : landuse)
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
                    width = std::fmax(2.5F * (1.0 / camera.zoom), width);
                }
                else if (kPrimary.find(tag->second) != kPrimary.end())
                {
                    lineColor = { 191, 117, 36,255 };
                    width = std::fmax(2.0F * (1.0 / camera.zoom), width);
                }
                else if (kSecondary.find(tag->second) != kSecondary.end())
                {
                    lineColor = { 0,0,0,255 };
                    width = std::fmax(1.0F * (1.0 / camera.zoom), width);
                }
                else if (kTertiary.find(tag->second) != kTertiary.end())
                {
                    lineColor = { 0,0,0,255 };
                    width = std::fmax(1.0F * (1.0 / camera.zoom), width);
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

                    Vector2 p1 = MercatorProjection(node1.location);
                    Vector2 p2 = MercatorProjection(node2.location);

                    DrawLineEx(
                        p1,
                        p2,
                        width,
                        lineColor
                    );
                }
            }
        }
    }

    if (!selected_path.empty())
    {
        for (size_t i = 0; i < selected_path.size() - 1; i++)
        {
            OSMNode& node1 = nodes[selected_path[i]];
            OSMNode& node2 = nodes[selected_path[i + 1]];

            Vector2 p1 = MercatorProjection(node1.location);
            Vector2 p2 = MercatorProjection(node2.location);

            float width = std::fmax(2.6F * (1.0 / camera.zoom), 0.2F);

            DrawLineEx(
                p1,
                p2,
                width,
                SKYBLUE
            );
        }
    }

    if (camera.zoom > 7.F)
    {
        for (MapObject& nodeObj : m_NodesToRender)
        {
            OSMNodeID& id = nodeObj.id;
            OSMNode& node = nodes[id];

            const AABB& bounds = nodeObj.bounds;

            if ((bounds.maxY < screenBounds.minY) || (bounds.minY > screenBounds.maxY) ||
                (bounds.maxX < screenBounds.minX) || (bounds.minX > screenBounds.maxX))
            {
                continue;
            }

            Vector2 p1 = MercatorProjection(node.location);

            float distToCursor = Vector2Distance(p1, settings.cursorPos);

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

    // for (const auto& node : m_pGraph->places) {
    //     auto current_node = m_pGraph->GetNode(node);

    //     Vector2 p1 = MercatorProjection(current_node.location);

    //     DrawCircleV(
    //         p1,
    //         10.F * (1.F / camera.zoom),
    //         GREEN
    //         );
    // }

    if (settings.drawQuadBounds)
    {
        std::vector<AABB> quadBounds;
        m_Tree.QueryQuads(screenBounds, &quadBounds, 20);
        for (const AABB& bounds : quadBounds)
        {
            DrawBounds(bounds, camera);
        }
    }
}

Polygon& OSMRenderer::CachePolygonSingle(OSMWayID wayId, const OSMWay& way)
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
        Vector2 p1 = MercatorProjection(n1.location);
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

void OSMRenderer::DrawBounds(const AABB& bounds, Camera2D camera)
{
    Vector2 p1 = MercatorProjection({bounds.minY, bounds.minX});
    Vector2 p2 = MercatorProjection({bounds.maxY, bounds.maxX});

    // Ensure rectangle has positive width/height and correct origin
    float rx = std::min(p1.x, p2.x);
    float ry = std::min(p1.y, p2.y);
    float rw = std::fabs(p2.x - p1.x);
    float rh = std::fabs(p2.y - p1.y);

    DrawRectangleLinesEx(
        { rx, ry, rw, rh },
        1.F * (1.F / camera.zoom),
        RED
    );
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

void QuadNode::Query(const AABB& range, MapObjects* foundNodes, LayeredMapObjects* foundWays, int depth) const
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
                foundWays->at(obj.layer).push_back(obj);
        }
    }

    if (!divided || depth == 0)
        return;

    nw->Query(range, foundNodes, foundWays, depth - 1);
    ne->Query(range, foundNodes, foundWays, depth - 1);
    sw->Query(range, foundNodes, foundWays, depth - 1);
    se->Query(range, foundNodes, foundWays, depth - 1);
}

void QuadNode::QueryQuads(const AABB& range, std::vector<AABB>* foundBounds, int depth) const
{
    if (!boundary.intersects(range))
        return;

    foundBounds->push_back(boundary);

    if (!divided || depth == 0)
        return;
    nw->QueryQuads(range, foundBounds, depth - 1);
    ne->QueryQuads(range, foundBounds, depth - 1);
    sw->QueryQuads(range, foundBounds, depth - 1);
    se->QueryQuads(range, foundBounds, depth - 1);
}

AABB GetScreenLocationBounds(Camera2D camera, float renderWidth, float renderHeight)
{
    Vector2 topLeft           = GetScreenToWorld2D({ 0,0 }, camera);
    Vector2 bottomRight       = GetScreenToWorld2D({ renderWidth, renderHeight }, camera);
    Coord   topLeftLatLon     = InverseMercatorProjection({topLeft.x, topLeft.y});
    Coord   bottomRightLatLon = InverseMercatorProjection({bottomRight.x, bottomRight.y});

    double minLat = bottomRightLatLon.lat;
    double maxLat = topLeftLatLon.lat;
    double minLon = topLeftLatLon.lon;
    double maxLon = bottomRightLatLon.lon;

    return AABB{minLon, minLat, maxLon, maxLat};
}