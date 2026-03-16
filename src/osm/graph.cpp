#include "graph.h"
#include "tags.h"
#include "tinyxml2.h"
#include "rlgl.h"
#include "spdlog/spdlog.h"

#include <algorithm>
#include <cmath>
#include <array>
#include <osmium/io/any_input.hpp>
#include <osmium/visitor.hpp>

#include "osm_handler.h"

OSMGraph::OSMGraph() : selectedNodeA(UINT32_MAX), selectedNodeB(UINT32_MAX)
{
}

bool OSMGraph::ParsePBF(const std::string& path)
{
    nodes.clear();
    ways.clear();

    nodes.reserve(10'000'000);
    ways.reserve(1'000'000);

    spdlog::info("Loading OSM PBF file from path: {}", path);

    osmium::io::Reader reader(path);

    OSMHandler handler(*this);

    osmium::apply(reader, handler);

    spdlog::info("Successfully loaded OSM PBF file, parsing...");

    reader.close();

    return true;
}


bool OSMGraph::ParseXML(std::string path)
{
    tinyxml2::XMLDocument doc;

    nodes.clear();
    ways.clear();

    nodes.reserve(10'000'000);
    ways.reserve(1'000'000);

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

            nodes.insert({ id, OSMNode{lat, lon} });

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
                way.nodes.push_back(ref);
                child = child->NextSiblingElement("nd");
            }
            
            ways.insert({id, way});
            element = element->NextSiblingElement("way");
        }
    }
    
    return true;
}

bool OSMGraph::load(const std::string& path)
{
    if (path.ends_with(".pbf"))
    {
        return ParsePBF(path);
    }
    if (path.ends_with(".xml") || path.ends_with(".osm"))
    {
        return ParseXML(path);
    }
    return false;
}

bool OSMGraph::BuildAdjList()
{
    adj_list.clear();
    
    spdlog::info("Building adjecency list...");

    for (const auto& wayPair : ways)
    {
        const OSMWay& way = wayPair.second;
        
        const std::vector<uint64_t>& nodes = way.nodes;

        auto highway = way.tags.find("highway");
        if (highway == way.tags.end())
            continue;

        if (kDrivableHighways.find(highway->second) == kDrivableHighways.end())
            continue;

        auto oneWayTag = way.tags.find("oneway");
        const bool oneWay = (oneWayTag != way.tags.end()) && (oneWayTag->second == "yes");

        for (size_t i = 0; i + 1 < nodes.size(); ++i)
        {
            uint64_t a = nodes[i];
            uint64_t b = nodes[i + 1];

            adj_list_dist.insert({a, INFINITY});
            adj_list_dist.insert({b, INFINITY});
            adj_list_prev.insert({a, 0xFFFFFFFF});
            adj_list_prev.insert({b, 0xFFFFFFFF});

            adj_list[a].push_back(b);

            if (!oneWay)
            {
                adj_list[b].push_back(a);
            }
        }
    }
    
    return true;
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


