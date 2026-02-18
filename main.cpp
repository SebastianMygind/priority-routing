
#include <print>
#include "raylib.h"
#include "Window.h"
#include "rlgl.h"
#include "raymath.h"
#include "tinyxml2.h"



#include <vector>
#include <unordered_map>


bool ParseData(std::string file);
std::string FetchData();

struct OSMNode 
{
    double lat;
    double lon;
};

struct OSMWay 
{
    std::vector<uint64_t> nodeRefs;
    std::unordered_map<std::string, std::string> tags;
};

std::unordered_map<uint64_t, OSMNode> nodes;
std::vector<OSMWay> ways;

int main() {
    std::println("Hello World!");

    std::string xml = FetchData();

    if (!ParseData(xml))
    {
        std::println("Failed to parse data");
        return 1;
	}

    

    auto window = Window("Routing Simulation");
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_MSAA_4X_HINT | FLAG_WINDOW_HIGHDPI);

    InitWindow(window.width, window.height, window.title.c_str());

    Camera2D camera = {0};
    camera.zoom = 1.0f;
    const float dpi = GetWindowScaleDPI().x;

    SetTargetFPS(GetMonitorRefreshRate(GetCurrentMonitor()));

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        const Vector2 mouseWorldPos = GetScreenToWorld2D(GetMousePosition(), camera);

        // Update
        if (IsWindowResized()) 
        {
            window.width = GetScreenWidth();
            window.height = GetScreenHeight();
        }

        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT))
        {
            Vector2 delta = GetMouseDelta();
            delta = Vector2Scale(delta, (-1.0f)/camera.zoom);
            camera.target = Vector2Add(camera.target, delta);
        }

        if (const float wheel = GetMouseWheelMove(); wheel != 0)
        {
            // Set the offset to where the mouse is
            camera.offset = GetMousePosition();

            // Set the target to match, so that the camera maps the world space point
            // under the cursor to the screen space point under the cursor at any zoom
            camera.target = mouseWorldPos;

            // Zoom increment
            // Uses log scaling to provide consistent zoom speed
            const float scale = 0.2f*wheel;
            camera.zoom = Clamp(expf(logf(camera.zoom)+scale), 0.025f, 1024.0f);
        }

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

        ClearBackground({230, 230, 230, 255});

        BeginMode2D(camera);
            // Draw the 3d grid, rotated 90 degrees and centered around 0,0
            // just so we have something in the XY plane
            rlPushMatrix();
                rlTranslatef(0, 25*50, 0);
                rlRotatef(90, 1, 0, 0);
                DrawGrid(100, 50);
            rlPopMatrix();

            Vector2 topLeft = GetScreenToWorld2D({ 0,0 }, camera);
            Vector2 bottomRight = GetScreenToWorld2D({ (float)window.width, (float)window.height }, camera);

            float minX = topLeft.x;
            float maxX = bottomRight.x;
            float minY = topLeft.y;
            float maxY = bottomRight.y;

            for (OSMWay& way : ways)
            {
                if (auto tag = way.tags.find("building"); tag != way.tags.end())
                {
                    std::vector<Vector2> points;

                    for (int i = 0; i < way.nodeRefs.size() - 1; i++)
                    {
                        OSMNode& n1 = nodes[way.nodeRefs[i]];
                        points.push_back({ (float)(n1.lon), (float)(-n1.lat * 1.8) });
                    }

                    DrawTriangleFan(points.data(), points.size(), MAROON);
                }
                else if (auto tag = way.tags.find("highway"); tag != way.tags.end())
                {
                    Color lineColor = { 100,100,100,255 };
					float width = 0.2f;
                   

                    if (tag->second == "motorway" || tag->second == "trunk" || tag->second == "motorway_link" || tag->second == "trunk_link")
                    {
                        lineColor = { 233,144,160,255 };
						width = 0.4f;
                    }
					else if (tag->second == "primary" || tag->second == "primary_link")
                    {
                        lineColor = { 253,215,161,255 };
                        width = 0.4f;
                    }
                    else if (tag->second == "secondary" || tag->second == "secondary_link")
                    {
                        lineColor = { 246,250,187,255 };
                        width = 0.3f;
                    }

                    for (int i = 0; i < way.nodeRefs.size() - 1; i++)
                    {
                        OSMNode& n1 = nodes[way.nodeRefs[i]];
                        OSMNode& n2 = nodes[way.nodeRefs[i + 1]];

                        Vector2 p1 = { (float)(n1.lon), (float)(-n1.lat * 1.8) };
                        Vector2 p2 = { (float)(n2.lon), (float)(-n2.lat * 1.8) };

                        bool outside =
                            p1.x < minX ||
                            p1.x > maxX ||
                            p1.y < minY ||
                            p1.y > maxY;

                        if (outside)
                            continue;

                        DrawLineEx(p1, p2, width, lineColor);
                    }
                }
            }


        EndMode2D();

        DrawCircle(window.width, window.height/2, 100, PURPLE);

        DrawCircleV(GetMousePosition(), 4, DARKGRAY);
        DrawTextEx(GetFontDefault(), TextFormat("[%.2f, %.2f]", mouseWorldPos.x, mouseWorldPos.y),
            Vector2Add(GetMousePosition(), Vector2{ -44, -24 }), 20, 2, BLACK);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}

bool ParseData(std::string xml)
{
    tinyxml2::XMLDocument doc;

    if (doc.Parse(xml.c_str()) != tinyxml2::XML_SUCCESS)
    {
        std::println("Failed to load xml");
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

            lat *= 5000.0;
            lon *= 5000.0;

            nodes.insert({ id, OSMNode{lat, lon} });

            element = element->NextSiblingElement("node");
        }
    }

    {
        tinyxml2::XMLElement* element = root->FirstChildElement("way");

        while (element)
        {
            OSMWay way = {};

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

            //tinyxml2::XMLElement* tag = element->FirstChildElement("tag");
            //if (tag)
            //    way.type = std::string(tag->Attribute("k"));

            ways.push_back(way);

            element = element->NextSiblingElement("way");
        }
    }
    return true;
}