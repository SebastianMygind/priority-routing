#include <cstdint>
#include <print>
#include <vector>

#include "Graph.h"
#include "PathFinding.h"
#include "raylib.h"
#include "Window.h"
#include "rlgl.h"
#include "raymath.h"
#include "spdlog/spdlog.h"
#include "raylib_logger.h"

int main() {

    Graph graph;

    SetTraceLogCallback(SPDLogger);
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_MSAA_4X_HINT | FLAG_WINDOW_HIGHDPI | FLAG_VSYNC_HINT);

    auto window = Window("Routing Simulation");

    InitWindow(window.width, window.height, window.title.c_str());
    // Setup Application logic at this stage.
    Camera2D camera = {0};
    camera.zoom = 1.0F;

    std::vector<Node> nodes = {{25, 25}, {50, 50}, {75, 75}, {75, 125}};
    std::vector<std::pair<uint32_t, uint32_t>> edges = {{0, 1}, {1, 2}, {2, 3}, {3, 0}};

    // Main game loop
    while (!WindowShouldClose())  // Detect window close button or ESC key
    {
        // Get the world point that is under the mouse
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
            delta = Vector2Scale(delta, -1.0F / camera.zoom);
            camera.target = Vector2Add(camera.target, delta);
        }

        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) 
        {
            for (Node& node : graph.nodes) {
                if (Vector2Distance({node.x, node.y}, mouseWorldPos) < 10) 
                {
                    if (graph.selected_node_a == 0xFFFFFFFF) {              // Click one, A
                        graph.selected_node_a = &node - graph.nodes.data();
                    } else if (graph.selected_node_b == 0xFFFFFFFF) {       // Click two, B, calculate path
                        graph.selected_node_b = &node - graph.nodes.data();
                        Djikstra(graph, graph.selected_node_a, graph.selected_node_b, graph.selected_path);
                    } else {                                                // Click three, reset
                        graph.selected_node_a = 0xFFFFFFFF;
                        graph.selected_node_b = 0xFFFFFFFF;
                        graph.selected_path.clear();
                    }
                    break;
                }
            }
        }

        if (const float wheel = GetMouseWheelMove(); wheel != 0) {
            // Set the offset to where the mouse is
            camera.offset = GetMousePosition();

            // Set the target to match, so that the camera maps the world space point
            // under the cursor to the screen space point under the cursor at any zoom
            camera.target = mouseWorldPos;

            // Zoom increment
            // Uses log scaling to provide consistent zoom speed
            const float scale = 0.2F * wheel;
            camera.zoom = Clamp(expf(logf(camera.zoom) + scale), 0.125F, 64.0F);
        }

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

        ClearBackground(RAYWHITE);

        BeginMode2D(camera);
        // Draw the 3d grid, rotated 90 degrees and centered around 0,0
        // just so we have something in the XY plane
        rlPushMatrix();
        rlTranslatef(0, 25 * 50, 0);
        rlRotatef(90, 1, 0, 0);
        DrawGrid(100, 50);
        rlPopMatrix();

        graph.DrawGraph();

        EndMode2D();

        DrawCircleV(GetMousePosition(), 4, DARKGRAY);

        DrawTextEx(
            GetFontDefault(),
            TextFormat("[%.2f, %.2f]", mouseWorldPos.x, mouseWorldPos.y),
            Vector2Add(GetMousePosition(), (Vector2){-44, -24}), 20, 2, BLACK
        );

        DrawTextEx(
            GetFontDefault(),
            TextFormat("Selected Node: \n A:%i, B:%i", graph.selected_node_a, graph.selected_node_b),
            {10, 10}, 20, 2, BLACK
        );

        EndDrawing();
    }

    CloseWindow();

    return 0;
}
