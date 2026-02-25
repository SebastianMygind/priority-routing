#include "Parser.h"
#include "tinyxml2.h"
#include "spdlog/spdlog.h"

bool ParseOSM(std::string path, Graph& out_graph)
{
    tinyxml2::XMLDocument doc;

    out_graph.nodes.clear();
    out_graph.edges.clear();

    if (doc.LoadFile(path.c_str()) != tinyxml2::XML_SUCCESS)
    {
        spdlog::error("Failed to parse/load xml file");
        return false;
    }

    tinyxml2::XMLNode* root = doc.FirstChildElement("osm");

    {
        tinyxml2::XMLElement* element = root->FirstChildElement("node");

        while (element)
        {
            const char* asd = element->Attribute("id");
            uint64_t id = std::stoull(element->Attribute("id"));
            double lat = std::stod(element->Attribute("lat"));
            double lon = std::stod(element->Attribute("lon"));

            lat -= 55.6539977;
            lon -= 12.5422305;

            lat *= 500000.0;
            lon *= 500000.0;

            out_graph.nodes.insert({ id, Node{(float)lat, (float)lon} });

            element = element->NextSiblingElement("node");
        }
    }

    {
        tinyxml2::XMLElement* element = root->FirstChildElement("way");

        while (element)
        {
            Edge way = {};

            tinyxml2::XMLElement* child = element->FirstChildElement("nd");
            while (child)
            {
                uint64_t ref = std::stoull(child->Attribute("ref"));
                way.nodeRefs.push_back(ref);
                child = child->NextSiblingElement("nd");
            }

            tinyxml2::XMLElement* childTag = element->FirstChildElement("tag");
            while (childTag)
            {
                way.tags.insert({ 
                    std::string(childTag->Attribute("k")), 
                    std::string(childTag->Attribute("v")) 
                });

                childTag = childTag->NextSiblingElement("tag");
            }

            if (auto tag = way.tags.find("highway"); tag != way.tags.end())
            {
                out_graph.edges.push_back(way);
            }

            // if (way.tags.second != "motorway" || tag->second != "trunk" || tag->second != "motorway_link" || tag->second != "trunk_link" || 
            //     way.tags->second != "primary" || tag->second != "primary_link" || tag->second != "secondary" || tag->second != "secondary_link")
            // {
 
            // }

            //tinyxml2::XMLElement* tag = element->FirstChildElement("tag");
            //if (tag)
            //    way.type = std::string(tag->Attribute("k"));

            

            element = element->NextSiblingElement("way");
        }
    }
    return true;
}