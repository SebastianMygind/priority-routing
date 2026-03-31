#include "../attribute_utils.h"
#include "../caching.h"
#include "../node_attributes.h"

#include <filesystem>
#include <float.h>
#include <math.h>
#ifdef _WIN32
#define NOUSER
#endif

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

OSMGraph::OSMGraph()
    : selectedNodeA(UINT32_MAX),
      selectedNodeB(UINT32_MAX) {}

bool OSMGraph::ParsePBF(const std::string& path) {
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

bool OSMGraph::ParseXML(std::string path) {
    tinyxml2::XMLDocument doc;

    nodes.clear();
    ways.clear();

    nodes.reserve(10'000'000);
    ways.reserve(1'000'000);

    spdlog::info("Loading OSM XML file from path: {}", path);

    if (doc.LoadFile(path.c_str()) != tinyxml2::XML_SUCCESS) {
        spdlog::error("Failed to parse/load xml file");
        return false;
    }

    spdlog::info("Successfully loaded OSM XML file, parsing...");

    tinyxml2::XMLNode* root = doc.FirstChildElement("osm");

    {
        tinyxml2::XMLElement* element = root->FirstChildElement("node");
        while (element) {
            uint64_t id = std::stoull(element->Attribute("id"));
            double lat = std::stod(element->Attribute("lat"));
            double lon = std::stod(element->Attribute("lon"));

            nodes.insert({id, OSMNode{lat, lon}});

            element = element->NextSiblingElement("node");
        }
    }

    {
        tinyxml2::XMLElement* element = root->FirstChildElement("way");
        while (element) {
            OSMWay way = {};

            uint64_t id = std::stoull(element->Attribute("id"));

            tinyxml2::XMLElement* childTag = element->FirstChildElement("tag");
            while (childTag) {
                way.tags.insert({std::string(childTag->Attribute("k")),
                                 std::string(childTag->Attribute("v"))});

                childTag = childTag->NextSiblingElement("tag");
            }

            // auto tag = way.tags.find("highway");
            // const bool isHighway = tag != way.tags.end();

            tinyxml2::XMLElement* child = element->FirstChildElement("nd");
            while (child) {
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

bool OSMGraph::load(const std::string& path) {
    if (path.ends_with(".pbf")) {
        return ParsePBF(path);
    }
    if (path.ends_with(".xml") || path.ends_with(".osm")) {
        return ParseXML(path);
    }
    return false;
}

bool OSMGraph::BuildAdjList() {
    adj_list.clear();

    spdlog::info("Getting node attributes...");

    const auto cachedTourismFile = GetNodeAttributes();

    spdlog::info("Building adjacency list...");

    for (const auto& wayPair : ways) {
        const OSMWay& way = wayPair.second;

        const std::vector<uint64_t>& nodes = way.nodes;

        auto highwayTag = way.tags.find("highway");
        auto oneWayTag = way.tags.find("oneway");
        auto speedTag = way.tags.find("maxspeed");

        if (highwayTag == way.tags.end()) continue;

        if (kDrivableHighways.find(highwayTag->second) ==
            kDrivableHighways.end())
            continue;

        const bool oneWay =
            (oneWayTag != way.tags.end()) && (oneWayTag->second == "yes");

        const double speed = (speedTag != way.tags.end())
                                 ? ParseMaxSpeed(speedTag->second)
                                 : GetDefaultSpeed(highwayTag->second);

        for (size_t i = 0; i + 1 < nodes.size(); ++i) {
            uint64_t a = nodes[i];
            uint64_t b = nodes[i + 1];

            adj_list_dist.insert({a, INFINITY});
            adj_list_dist.insert({b, INFINITY});
            adj_list_prev.insert({a, 0xFFFFFFFF});
            adj_list_prev.insert({b, 0xFFFFFFFF});

            double tourismCostA = cachedTourismFile.at(a).attributes.at("tourism");
            double tourismCostB = cachedTourismFile.at(b).attributes.at("tourism");

            adj_list[a].push_back({b, tourismCostB});

            if (!oneWay) {
                adj_list[b].push_back({a, tourismCostA});
            }
        }
    }

    return true;
}

OSMNodeID OSMGraph::StringToNode(const std::string& input) {
    try 
    {
        OSMNodeID node = std::stoull(input);
        if (nodes.find(node) == nodes.end()) 
        {
            throw std::runtime_error("Node not found");
        }
        return node;
    } 
    catch (const std::exception& e) 
    {
        return 0xFFFFFFFF;
    }
}


std::vector<OSMNodeID> OSMGraph::getNodesWithTourism() const {
    std::vector<OSMNodeID> nodesWithTourism;

    for (const auto& [nodeID, node] : nodes) {
        if (auto tag = node.tags.find("tourism"); tag != node.tags.end()) {
            nodesWithTourism.push_back(nodeID);
        }
    }

    return nodesWithTourism;
}

std::pair<OSMNodeID, double> OSMGraph::getNearestNode(
    const std::vector<OSMNodeID>& nodeGroup, const OSMNodeID center) const {
    std::pair<OSMNodeID, double> closestNode = {0, INFINITY};

    const auto centerNode = GetNode(center);

    for (const auto& node : nodeGroup) {
        auto distance = Haversine(centerNode, GetNode(node));

        if (distance < closestNode.second) {
            closestNode = {node, distance};
        }
    }

    return closestNode;
}

std::unordered_map<std::string, std::pair<OSMNodeID, double>> GenerateAttrMap (attr_map_t& attrInfo) {
    std::unordered_map<std::string, std::pair<OSMNodeID, double>> map;

    for (const auto& [attrName, tuple] : attrInfo) {
        const auto goal = std::get<1>(tuple);

        double startValue = 0;
        if (goal == Goal::Maximize) {
            startValue = DBL_MIN;
        } else {
            startValue = DBL_MAX;
        }

        map[attrName] = {0,startValue};
    }

    return map;
}

file_format_t OSMGraph::GenerateNodeAttributes(attr_map_t attrInfo) {
    file_format_t fileAtributes;

    for (const auto& [wayNodes, tags] : ways | std::views::values) {
        auto highwayTag = tags.find("highway");
        auto oneWayTag = tags.find("oneway");
        auto speedTag = tags.find("maxspeed");

        if (highwayTag == tags.end()) continue;

        if (!kDrivableHighways.contains(highwayTag->second)) continue;

        const double speed = (speedTag != tags.end())
                                 ? ParseMaxSpeed(speedTag->second)
                                 : GetDefaultSpeed(highwayTag->second);

        for (const auto node : wayNodes) {
            if (fileAtributes.contains(node)) {
                continue;
            }

            auto bestResults = GenerateAttrMap(attrInfo);


            for (const auto& [attrName, tuple] : attrInfo) {
                const auto func = std::get<0>(tuple);
                const auto& searchSpace = std::get<2>(tuple);
                auto& currentAttr = bestResults[attrName];
                // Call the function to update the best result.
                func(nodes, searchSpace, node, currentAttr);
            }

            auto nodeAttributes = NodeAttributes();

            for (const auto& [name, val] : bestResults) {
                nodeAttributes.attributes[name] = val.second;
            }
            fileAtributes[node] = nodeAttributes;
        }
    }

    return fileAtributes;
}

file_format_t OSMGraph::GetNodeAttributes() {
    file_format_t nodeAttributes;

    const std::vector<OSMNodeID>& tourismNodes = getNodesWithTourism();

    attr_map_t attributeMap;

    attributeMap["tourism"] = std::make_tuple(TourismFunc, Goal::Minimize, tourismNodes);

    const auto attrCount = attributeMap.size();
    const auto attrHash = GetAttrHash(attributeMap);

    // Check if cache has valid info
    const auto* const filepath = "../cache/attributes.cache";

    const auto dataSetHash = DataSetHash(pathForOSM);
    const auto combinedHash = CombineHash(attrHash, dataSetHash);

    if (cacheIsValid(filepath, combinedHash)) {
        spdlog::info(
            "cached tourism file exists, loading from storage");

        if (auto attrResult = ReadFromCache(filepath, attributeMap); attrResult.has_value()) {
            nodeAttributes = attrResult.value();
        } else {
            spdlog::critical(attrResult.error());
            exit(-1);
        }
        return nodeAttributes;
    }

    nodeAttributes = GenerateNodeAttributes(attributeMap);

    const auto writeRes =
        WriteToCache(filepath, nodeAttributes, combinedHash, attrCount);

    if (!writeRes.has_value()) {
        spdlog::critical(writeRes.error());
        exit(-1);
    }

    return nodeAttributes;
}

Vector2 MercatorProjection(double lat, double lon) {
    double centerLatitude = 55.6539977;
    double centerLongitude = 12.5422305;

    // constexpr double _PI = 3.14159265358979323846;

    // Clamp latitude to avoid infinity
    lat = std::clamp(lat, -85.05112878, 85.05112878);
    centerLatitude = std::clamp(centerLatitude, -85.05112878, 85.05112878);

    // Convert to radians
    double latRad = lat * PI / 180.0;
    double lonRad = lon * PI / 180.0;
    double centerLatRad = centerLatitude * PI / 180.0;
    double centerLonRad = centerLongitude * PI / 180.0;

    // Proper Mercator projection
    double x = lonRad;
    double y = std::log(std::tan(PI / 4.0 + latRad / 2.0));

    double cx = centerLonRad;
    double cy = std::log(std::tan(PI / 4.0 + centerLatRad / 2.0));

    return {
        static_cast<float>((x - cx) * 500000.0),
        static_cast<float>((cy - y) * 500000.0)  // flip Y for screen coords
    };
}

Coord InverseMercatorProjection(float worldX, float worldY) {
    double centerLatitude = 55.6539977;
    double centerLongitude = 12.5422305;

    constexpr double scale = 500000.0;

    // Clamp center latitude (same as forward projection)
    centerLatitude = std::clamp(centerLatitude, -85.05112878, 85.05112878);

    double centerLatRad = centerLatitude * PI / 180.0;
    double centerLonRad = centerLongitude * PI / 180.0;

    // Reconstruct Mercator center Y
    double cy = std::log(std::tan(PI / 4.0 + centerLatRad / 2.0));

    // --- Invert screen transform ---
    double x = (worldX / scale) + centerLonRad;
    double y = cy - (worldY / scale);

    // --- Invert Mercator ---
    double lonRad = x;
    double latRad = 2.0 * std::atan(std::exp(y)) - PI / 2.0;

    return {latRad * 180.0 / PI, lonRad * 180.0 / PI};
}

const double EARTH_RADIUS = 6371000.0;  // in meters

static double KmhToMS(double kmh) { return kmh * 1000.0 / 3600.0; }

static double MphToMS(double mph) { return mph * 1609.34 / 3600.0; }

double ParseMaxSpeed(const std::string& speedStr) {
    if (speedStr.empty()) return 50.0;  // fallback default

    std::stringstream ss(speedStr);
    double value;
    ss >> value;

    if (speedStr.find("mph") != std::string::npos)
        return value * 1.60934;  // mph → km/h

    return value;  // assume km/h
}

double GetDefaultSpeed(const std::string& highway) {
    if (highway == "motorway") return 130;
    if (highway == "trunk") return 110;
    if (highway == "primary") return 80;
    if (highway == "secondary") return 70;
    if (highway == "tertiary") return 60;
    if (highway == "residential") return 50;
    if (highway == "service") return 30;

    return 50;
}

static double DegToRad(double deg) { return deg * PI / 180.0; }

double Haversine(const OSMNode& a, const OSMNode& b) {
    double lat1 = DegToRad(a.lat);
    double lat2 = DegToRad(b.lat);
    double dLat = lat2 - lat1;
    double dLon = DegToRad(b.lon - a.lon);

    double h = std::sin(dLat / 2) * std::sin(dLat / 2) +
               std::cos(lat1) * std::cos(lat2) * std::sin(dLon / 2) *
                   std::sin(dLon / 2);

    double c = 2 * std::atan2(std::sqrt(h), std::sqrt(1 - h));

    return EARTH_RADIUS * c;
}

double EuclideanDistance(const OSMNode& a, const OSMNode& b) {
    double xDiff = (b.lon - a.lon) * cos((a.lat + b.lat) / 2);
    double yDiff = (b.lat - a.lat);

    return std::sqrt(pow(xDiff, 2) * pow(yDiff, 2));
}
