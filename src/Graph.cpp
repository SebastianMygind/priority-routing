#include "Graph.h"
#include "Tags.h"

#include <algorithm>
#include <cmath>

Graph::Graph() : selected_node_a(UINT32_MAX), selected_node_b(UINT32_MAX)
{

}

void Graph::DrawGraph(Camera2D camera, float screenWidth, float screenHeight)
{
    Vector2 topLeft     = GetScreenToWorld2D({ 0,0 }, camera);
    Vector2 bottomRight = GetScreenToWorld2D({ screenWidth, screenHeight }, camera);

    Node topLeftLatLon     = InverseMercatorProjection(topLeft.x, topLeft.y, screenHeight, screenWidth);
    Node bottomRightLatLon = InverseMercatorProjection(bottomRight.x, bottomRight.y, screenHeight, screenWidth);

    double minLat = bottomRightLatLon.lat;
    double maxLat = topLeftLatLon.lat;
    double minLon = topLeftLatLon.lon;
    double maxLon = bottomRightLatLon.lon;

    for (const Way& way : ways) 
    {
        Node& firstNode = nodes[way.nodeRefs.front()];
        Node& lastNode = nodes[way.nodeRefs.back()];

        if ((firstNode.lat < minLat && lastNode.lat < minLat) ||
            (firstNode.lat > maxLat && lastNode.lat > maxLat) ||
            (firstNode.lon < minLon && lastNode.lon < minLon) ||
            (firstNode.lon > maxLon && lastNode.lon > maxLon))
        {
            continue;
        }

        if (auto tag = way.tags.find("highway"); tag != way.tags.end())
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
                Node& node1 = nodes[way.nodeRefs[i]];
                Node& node2 = nodes[way.nodeRefs[i + 1]];

                if ((node1.lat < minLat && node2.lat < minLat) ||
                    (node1.lat > maxLat && node2.lat > maxLat) ||
                    (node1.lon < minLon && node2.lon < minLon) ||
                    (node1.lon > maxLon && node2.lon > maxLon))
                {
                    continue;
                }

                Vector2 p1 = MercatorProjection(node1.lat, node1.lon, screenHeight, screenWidth);
                Vector2 p2 = MercatorProjection(node2.lat, node2.lon, screenHeight, screenWidth);
                    
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
    for (auto& node : nodes)
    {
        if (node.second.lon < minLon || node.second.lon > maxLon ||
            node.second.lat < minLat || node.second.lat > maxLat)
        {
            continue;
        }

        Vector2 p1 = MercatorProjection(node.second.lat, node.second.lon, screenHeight, screenWidth);

        bool isSelected = (node.first == selected_node_a || node.first == selected_node_b);
        DrawCircleV(
            p1,
            isSelected ? std::fmax(2.5F * (1.0 / camera.zoom), 0.1F) : 0.1F, 
            isSelected ? SKYBLUE : MAROON
        );
    }
}



Vector2 MercatorProjection(double lat, double lon, float screenHeight, float screenWidth)
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

Node InverseMercatorProjection(
    float screenX,
    float screenY,
    float screenHeight,
    float screenWidth)
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
    double x = (screenX / scale) + centerLonRad;
    double y = cy - (screenY / scale);

    // --- Invert Mercator ---
    double lonRad = x;
    double latRad = 2.0 * std::atan(std::exp(y)) - PI / 2.0;

    return {
        latRad * 180.0 / PI,
        lonRad * 180.0 / PI
    };
}