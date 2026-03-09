#include <cstdint>
#include <print>
#include <vector>
#include <thread>

#include "osm/graph.h"
#include "osm/renderer.h"
#include "path_finder.h"
#include "raylib.h"
#include "Window.h"
#include "rlgl.h"
#include "raymath.h"
#include "spdlog/spdlog.h"
#include "raylib_logger.h"
#include "user_interface.h"

int main() {

    UIState uiState;

    OSMGraph graph;
    OSMRenderer renderer(&graph);

    ParseOSM("../data/Copenhagen.osm", graph);

    renderer.BuildQuadTree();

    SetTraceLogCallback(SPDLogger);
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_MSAA_4X_HINT | FLAG_WINDOW_HIGHDPI | FLAG_VSYNC_HINT);


    auto window = Window("Routing Simulation");

    InitWindow(window.width, window.height, window.title.c_str());
    // Setup Application logic at this stage.
    Camera2D camera = {};
    camera.zoom = 1.0F;

#ifdef __APPLE__
    Vector2 dpi = {1.0, 1.0};
#else
    Vector2 dpi = GetWindowScaleDPI();
#endif

    while (!WindowShouldClose())
    {
        // Get the world point that is under the mouse
        const Vector2 mouseWorldPos = GetScreenToWorld2D(GetMousePosition() * dpi, camera);

        // Update
        if (IsWindowResized()) 
        {
            window.width = static_cast<int>(static_cast<float>(GetScreenWidth()) / dpi.x);
            window.height = static_cast<int>(static_cast<float>(GetScreenHeight())/ dpi.y);
        }

        if (IsKeyPressed(KEY_D)) {
            window.showDebug = !window.showDebug;
        }

        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) 
        {
            Vector2 delta = GetMouseDelta();
            delta = Vector2Scale(delta, -1.0F / camera.zoom);
            camera.target = Vector2Add(camera.target, delta);
        }

        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) 
        {
            Vector2 topLeft           = GetScreenToWorld2D({ 0,0 }, camera);
            Vector2 bottomRight       = GetScreenToWorld2D({ (float)window.width, (float)window.height }, camera);
            Coord   topLeftLatLon     = InverseMercatorProjection(topLeft.x, topLeft.y);
            Coord   bottomRightLatLon = InverseMercatorProjection(bottomRight.x, bottomRight.y);

            double minLat = bottomRightLatLon.lat;
            double maxLat = topLeftLatLon.lat;
            double minLon = topLeftLatLon.lon;
            double maxLon = bottomRightLatLon.lon;

            std::vector<MapObject> visibleNodes;
            renderer.tree.Query({minLon, minLat, maxLon, maxLat}, &visibleNodes, nullptr);

            for (MapObject& obj : visibleNodes) {
                OSMNode& node = graph.nodes[obj.id];
                if (Vector2Distance(MercatorProjection(node.lat, node.lon), mouseWorldPos) < 0.2F)
                {
                    if (graph.selected_node_a == 0xFFFFFFFF) {              // Click one, A
                        graph.selected_node_a = obj.id;
                    } else if (graph.selected_node_b == 0xFFFFFFFF) {       // Click two, B, calculate path
                        graph.selected_node_b = obj.id;
                        PathFinder(graph, uiState.modelSelection);
                    } else {                                                // Click three, reset
                        graph.selected_node_a = 0xFFFFFFFF;
                        graph.selected_node_b = 0xFFFFFFFF;
                        graph.selected_path.clear();
                    }
                    break;
                }
            }
        }

        if (const float wheel = GetMouseWheelMove(); wheel != 0) 
        {
            camera.offset = GetMousePosition() * dpi;
            camera.target = mouseWorldPos;

            const float scale = 0.2F * wheel;
            camera.zoom = Clamp(expf(logf(camera.zoom) + scale), 0.125F, 64.0F);
        }

        BeginDrawing();

        ClearBackground(RAYWHITE);

        BeginMode2D(camera);
        renderer.DrawGraph(camera, (float)window.width * dpi.x, (float)window.height * dpi.y);
        EndMode2D();

        DrawUserInterface(window, mouseWorldPos, graph, uiState);

        EndDrawing();
    }

    CloseWindow();

    return 0;
}
