#include "graph.h"
#include "tags.h"
#include "tinyxml2.h"
#include "rlgl.h"
#include "spdlog/spdlog.h"

#include <algorithm>
#include <cmath>
#include <array>

OSMGraph::OSMGraph() : selected_node_a(UINT32_MAX), selected_node_b(UINT32_MAX)
{
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



bool ParseOSM(std::string path, OSMGraph& out_graph)
{
    tinyxml2::XMLDocument doc;

    out_graph.nodes.clear();
    out_graph.ways.clear();

    out_graph.nodes.reserve(10'000'000);
    out_graph.ways.reserve(1'000'000);

    spdlog::info("Loading OSM XML file from path: {}", path);

    if (doc.LoadFile(path.c_str()) != tinyxml2::XML_SUCCESS)
    {
        spdlog::error("Failed to parse/load xml file");
        return false;
    }

    spdlog::info("Successfully loaded OSM XML file, parsing...");

    tinyxml2::XMLNode* root = doc.FirstChildElement("osm");

    {
        tinyxml2::XMLElement* element = root->FirstChildElement("node");
        while (element)
        {
            uint64_t id = std::stoull(element->Attribute("id"));
            double lat = std::stod(element->Attribute("lat"));
            double lon = std::stod(element->Attribute("lon"));

            out_graph.nodes.insert({ id, OSMNode{lat, lon} });

            element = element->NextSiblingElement("node");
        }
    }

    {
        tinyxml2::XMLElement* element = root->FirstChildElement("way");
        while (element)
        {
            OSMWay way = {};

            uint64_t id = std::stoull(element->Attribute("id"));

            tinyxml2::XMLElement* childTag = element->FirstChildElement("tag");
            while (childTag)
            {
                way.tags.insert({ 
                    std::string(childTag->Attribute("k")), 
                    std::string(childTag->Attribute("v")) 
                });

                childTag = childTag->NextSiblingElement("tag");
            }

            //auto tag = way.tags.find("highway");
            //const bool isHighway = tag != way.tags.end();

            tinyxml2::XMLElement* child = element->FirstChildElement("nd");
            while (child)
            {
                uint64_t ref = std::stoull(child->Attribute("ref"));
                way.nodeRefs.push_back(ref);
                child = child->NextSiblingElement("nd");
            }
            
            out_graph.ways.insert({id, way});
            element = element->NextSiblingElement("way");
        }
    }
    
    return true;
}
