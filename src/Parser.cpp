#include "Parser.h"
#include "tinyxml2.h"
#include "spdlog/spdlog.h"

#include <algorithm>

bool ParseOSM(std::string path, Graph& out_graph)
{
    tinyxml2::XMLDocument doc;

    out_graph.nodes.clear();
    out_graph.ways.clear();

    if (doc.LoadFile(path.c_str()) != tinyxml2::XML_SUCCESS)
    {
        spdlog::error("Failed to parse/load xml file");
        return false;
    }

    tinyxml2::XMLNode* root = doc.FirstChildElement("osm");

    std::unordered_map<uint64_t, Node> temp_nodes;

    {
        tinyxml2::XMLElement* element = root->FirstChildElement("node");
        while (element)
        {
            uint64_t id = std::stoull(element->Attribute("id"));
            double lat = std::stod(element->Attribute("lat"));
            double lon = std::stod(element->Attribute("lon"));

            temp_nodes.insert({ id, Node{lat, lon} });
            element = element->NextSiblingElement("node");
        }
    }

    {
        tinyxml2::XMLElement* element = root->FirstChildElement("way");

        while (element)
        {
            Way way = {};

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

                Node& node = temp_nodes[ref];

                if (isHighway)
                    out_graph.nodes.insert({ref, node});

                child = child->NextSiblingElement("nd");
            }
            
            if (isHighway)
                out_graph.ways.push_back(way);

            element = element->NextSiblingElement("way");
        }
    }
    
    return true;
}