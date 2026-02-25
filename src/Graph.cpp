#include "Graph.h"

#include <algorithm>

Graph::Graph() : selected_node_a(UINT32_MAX), selected_node_b(UINT32_MAX)
{

}

void Graph::DrawGraph(Camera2D camera, float screenWidth, float screenHeight)
{
    Vector2 topLeft = GetScreenToWorld2D({ 0,0 }, camera);
    Vector2 bottomRight = GetScreenToWorld2D({ screenWidth, screenHeight }, camera);

    float minX = topLeft.x;
    float maxX = bottomRight.x;
    float minY = topLeft.y;
    float maxY = bottomRight.y;

    for (const Way& way : ways) 
    {
        for (uint32_t i = 0; i < way.nodeRefs.size() - 1; i++)
        {
            Node& node1 = nodes[way.nodeRefs[i]];
            Node& node2 = nodes[way.nodeRefs[i + 1]];

            Vector2 p1 = MercatorProjection(node1.lat, node1.lon, screenHeight, screenWidth);
            Vector2 p2 = MercatorProjection(node2.lat, node2.lon, screenHeight, screenWidth);

            bool outside =
                p1.x < minX ||
                p1.x > maxX ||
                p1.y < minY ||
                p1.y > maxY;

            if (outside)
                continue;

            bool inPath =
                std::find(selected_path.begin(), selected_path.end(), way.nodeRefs[i]) != selected_path.end() &&
                std::find(selected_path.begin(), selected_path.end(), way.nodeRefs[i + 1]) != selected_path.end();

            DrawLineEx(
                p1, 
                p2, 
                inPath ? std::fmax(2.5F * (1.0 / camera.zoom), 0.1F) : 0.1F, 
                inPath ? SKYBLUE : GRAY
            );
        }
    }

    // Draw node
    for (auto& node : nodes)
    {
        Vector2 p1 = MercatorProjection(node.second.lat, node.second.lon, screenHeight, screenWidth);

        bool outside =
            p1.x < minX ||
            p1.x > maxX ||
            p1.y < minY ||
            p1.y > maxY;

        if (outside)
            continue;

        bool isSelected = (node.first == selected_node_a || node.first == selected_node_b);
        DrawCircleV(
            MercatorProjection(node.second.lat, node.second.lon, screenHeight, screenWidth),
            0.1F, 
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
