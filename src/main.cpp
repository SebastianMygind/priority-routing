#include <cstdint>
#include <print>
#include <vector>
#include <thread>

#include "osm/graph.h"
#include "osm/renderer.h"
#include "path_finder.h"
#include "raylib.h"
#include "Window.h"
#include "raymath.h"
#include "spdlog/spdlog.h"
#include "raylib_logger.h"
#include "user_interface.h"
#include "widgets/text.h"



int main() {

    UIState uiState;

    bool globalKeyboardIsLocked = false;

    OSMGraph graph;
    graph.load("../data/copenhagen.osm.pbf");
    graph.BuildAdjList();

    OSMRenderer renderer(&graph); 
    renderer.BuildQuadTree();

    SetTraceLogCallback(SPDLogger);
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_MSAA_4X_HINT | FLAG_WINDOW_HIGHDPI | FLAG_VSYNC_HINT);


    auto window = Window("Routing Simulation");

    InitWindow(window.width, window.height, window.title.c_str());
    // Setup Application logic at this stage.
    SetupFontConfig();

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
            // We divide here because some OS (Linux) return physical size on GetScreenWidth(). On others the dpi is 1.F
            window.width = static_cast<int>(static_cast<float>(GetScreenWidth()) / dpi.x);
            window.height = static_cast<int>(static_cast<float>(GetScreenHeight()) / dpi.y);
        }

        if (IsKeyPressed(KEY_D) && !globalKeyboardIsLocked) {
            window.showDebug = !window.showDebug;
        }

        if (IsKeyPressed(KEY_V) && !globalKeyboardIsLocked) {
            window.showDebugVisuals = !window.showDebugVisuals;
        }

        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) && GetMousePosition().x > 300) 
        {
            Vector2 delta = GetMouseDelta();
            delta = Vector2Scale(delta, -1.0F / camera.zoom);
            camera.target = Vector2Add(camera.target, delta);
        }

        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && camera.zoom > 7.F)
        {
            AABB bounds = GetScreenLocationBounds(camera, (float)window.width * dpi.x, (float)window.height * dpi.y);

            std::vector<MapObject> visibleNodes;
            renderer.m_Tree.Query(bounds, &visibleNodes, nullptr, 20);

            for (MapObject& obj : visibleNodes) 
            {
                OSMNode node = graph.GetNode(obj.id);
                if (Vector2Distance(MercatorProjection(node.lat, node.lon), mouseWorldPos) < 0.2F)
                {
                    if (graph.GetNodeA() == 0xFFFFFFFF) {              // Click one, A
                        graph.SetNodeA(obj.id);
                    } else if (graph.GetNodeB() == 0xFFFFFFFF) {       // Click two, B, calculate path
                        graph.SetNodeB(obj.id);
                        PathFinder(graph, uiState.modelSelection);
                    } else {                                                // Click three, reset
                        graph.SetNodeA(0xFFFFFFFF);
                        graph.SetNodeB(0xFFFFFFFF);
                        graph.ClearPath();
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

        static OSMRendererSettings settings;
        settings.screenWidth = (float)window.width * dpi.x;
        settings.screenHeight = (float)window.height * dpi.y;
        settings.drawObjBounds = false;
        settings.drawQuadBounds = window.showDebugVisuals;
        settings.cursorPos = mouseWorldPos;

        renderer.DrawGraph(camera, settings);

        EndMode2D();

        auto [lat, lon] = InverseMercatorProjection(mouseWorldPos.x, mouseWorldPos.y);
        DrawUserInterface(
            window,
            { .x=static_cast<float>(lon), .y=static_cast<float>(lat) },
            graph,
            renderer,
            uiState,
            globalKeyboardIsLocked
        );

        EndDrawing();
    }

    CloseWindow();

    return 0;
}
