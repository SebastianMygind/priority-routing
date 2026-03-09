#include "Parser.h"
#include "tinyxml2.h"
#include "spdlog/spdlog.h"

#include <algorithm>

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

            auto tag = way.tags.find("highway");
            const bool isHighway = tag != way.tags.end();

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
