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

int main() {

    OSMGraph graph;
    graph.load(graph.pathForOSM);
    graph.Build2DTree();
    if (!graph.BuildAdjList()) {
        return -1;
    }

    OSMRenderer renderer(&graph);
    renderer.BuildQuadTree();

    SetTraceLogCallback(SPDLogger);
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_MSAA_4X_HINT | FLAG_WINDOW_HIGHDPI | FLAG_VSYNC_HINT);

    auto window = Window("Routing Simulation");

    InitWindow(window.width, window.height, window.title.c_str());
    // UI must be initialized after creating the window.
    UserInterface ui;
    ui.SetDebugTotalNodes(graph.GetNodeCount());
    ui.SetDebugTotalWays(graph.GetWayCount());
    ui.SetupUI("../fonts/JetBrainsMono-Regular.ttf");

    Camera2D camera = {};
    camera.zoom = 1.0F;

#ifndef __APPLE__
    window.dpi = GetWindowScaleDPI();
#endif

    while (!WindowShouldClose())
    {
        // Get the world point that is under the mouse
        const Vector2 mouseWorldPos = GetScreenToWorld2D(GetMousePosition() * window.dpi, camera);

        // Update
        if (IsWindowResized()) 
        {
            // We divide here because some OS (Linux) return physical size on GetScreenWidth(). On others the dpi is 1.F
            window.width = static_cast<int>(static_cast<float>(GetScreenWidth()) / window.dpi.x);
            window.height = static_cast<int>(static_cast<float>(GetScreenHeight()) / window.dpi.y);
        }
        
        ui.UpdateLockState();
 
        if (!ui.KeyboardInUI())
        {
            if (IsKeyPressed(KEY_U)) { ui.ToggleUI();         }
            if (IsKeyPressed(KEY_D)) { ui.ToggleDebug();      }
            if (IsKeyPressed(KEY_V)) { renderer.ToggleQuad(); }
            if (IsKeyPressed(KEY_P)) { renderer.CyclePOI(); ui.SetPOIText(renderer.GetPOIText());  }
        }

        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) && !ui.MouseInUI())
        {
            Vector2 delta = GetMouseDelta();
            delta = Vector2Scale(delta, -1.0F / camera.zoom);
            camera.target = Vector2Add(camera.target, delta);
        }

        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && !ui.MouseInUI() && camera.zoom > 7.F)
        {
            AABB bounds = GetScreenLocationBounds(
                camera,
                static_cast<float>(window.width) * window.dpi.x,
                static_cast<float>(window.height) * window.dpi.y
            );

            LayeredMapObjects visibleNodes(4);
            renderer.m_Tree.Query(bounds, &visibleNodes, nullptr, 20);

            for (MapObject& obj : visibleNodes[0]) 
            {
                OSMNode node = graph.GetNode(obj.id);
                if (Vector2Distance(MercatorProjection(node.location), mouseWorldPos) < 0.2F)
                {
                    if (graph.GetNodeA() == 0xFFFFFFFF)                  // Click one, A
                    {
                        graph.SetNodeA(obj.id);
                        ui.SetOrigin(std::format("{}", obj.id));
                    } 
                    else if (graph.GetNodeB() == 0xFFFFFFFF)            // Click two, B
                    {
                        graph.SetNodeB(obj.id);
                        ui.SetDestination(std::format("{}", obj.id));
                    } 
                    else                                                // Click three, reset
                    {                                           
                        graph.SetNodeA(0xFFFFFFFF);
                        graph.SetNodeB(0xFFFFFFFF);
                        graph.ClearPath();
                        ui.SetOrigin("");
                        ui.SetDestination("");
                    }

                    if (graph.GetNodeA() != 0xFFFFFFFF && graph.GetNodeB() != 0xFFFFFFFF) 
                    {
                        PathFinder(graph, ui);
                    }
                    
                    break;
                }
            }
        }

        if (ui.IsUpdated())
        {
            graph.ClearPath();
            graph.SetNodeA(graph.StringToNode(ui.GetOrigin()));
            graph.SetNodeB(graph.StringToNode(ui.GetDestination()));
            
            if (graph.GetNodeA() != 0xFFFFFFFF && graph.GetNodeB() != 0xFFFFFFFF) 
            {
                PathFinder(graph, ui);
            }
        }

        if (const float wheel = GetMouseWheelMove(); wheel != 0) 
        {
            camera.offset = GetMousePosition() * window.dpi;
            camera.target = mouseWorldPos;

            const float scale = 0.2F * wheel;
            camera.zoom = Clamp(expf(logf(camera.zoom) + scale), 0.125F, 64.0F);
        }

        // Prepare renderer
        renderer.UpdateGraph(camera, window, mouseWorldPos);

        // Prepare UI
        Coord coord = InverseMercatorProjection(mouseWorldPos);
        ui.SetDebugMouseCoords(static_cast<float>(coord.lat), static_cast<float>(coord.lon));
        ui.SetDebugRenderedWays(renderer.GetWayRenderCount());
        ui.SetDebugRenderNodes(renderer.GetNodeRenderCount());

        BeginDrawing();

            ClearBackground(RAYWHITE);

            BeginMode2D(camera);
                renderer.DrawGraph();
            EndMode2D();

            ui.DrawUserInterface(window);

        EndDrawing();
    }

    CloseWindow();

    renderer.FinishThread();

    return 0;
}
