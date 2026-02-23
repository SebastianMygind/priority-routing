#include <print>

#include "Node.h"
#include "raylib.h"
#include "Window.h"
#include "rlgl.h"
#include "raymath.h"
#include "spdlog/spdlog.h"

int main() {

    std::vector<Node> nodes = {{25, 25}, {50, 50}, {75, 75}, {75, 125}};
    std::vector<std::pair<uint32_t, uint32_t>> edges = {{0, 1}, {1, 2}, {2, 3}, {3, 0}};


    auto window = Window("Routing Simulation");
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_MSAA_4X_HINT | FLAG_WINDOW_HIGHDPI);

    InitWindow(window.width, window.height, window.title.c_str());

    Camera2D camera = {0};
    camera.zoom = 1.0F;

    SetTargetFPS(GetMonitorRefreshRate(GetCurrentMonitor()));

    spdlog::error("Test error");

    // Main game loop
    while (!WindowShouldClose()) // Detect window close button or ESC key
    {
        // Update
        if (IsWindowResized()) {
            window.width = GetScreenWidth();
            window.height = GetScreenHeight();
        }

        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
            Vector2 delta = GetMouseDelta();
            delta = Vector2Scale(delta, (-1.0f) / camera.zoom);
            camera.target = Vector2Add(camera.target, delta);
        }

        if (const float wheel = GetMouseWheelMove(); wheel != 0) {
            // Get the world point that is under the mouse
            const Vector2 mouseWorldPos = GetScreenToWorld2D(GetMousePosition(), camera);

            // Set the offset to where the mouse is
            camera.offset = GetMousePosition();

            // Set the target to match, so that the camera maps the world space point
            // under the cursor to the screen space point under the cursor at any zoom
            camera.target = mouseWorldPos;

            // Zoom increment
            // Uses log scaling to provide consistent zoom speed
            const float scale = 0.2f * wheel;
            camera.zoom = Clamp(expf(logf(camera.zoom) + scale), 0.125f, 64.0f);
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

        // Draw a reference circle
        // DrawCircle(0, 0, 50, MAROON);

        // Draw edges

        for (const std::pair<uint32_t, uint32_t>& edge : edges) {
            Node &node1 = nodes[edge.first];
            Node &node2 = nodes[edge.second];

            DrawLineEx({node1.x, node1.y}, {node2.x, node2.y}, 3, BLACK);
        }

        // Draw vertices
        for (const Node &node : nodes) {
            DrawCircle(static_cast<int>(node.x), static_cast<int>(node.y), 10.F, MAROON);
        }

        EndMode2D();

        DrawCircle(window.width, window.height / 2, 100, PURPLE);

        DrawCircleV(GetMousePosition(), 4, DARKGRAY);
        const Vector2 dpi = GetWindowScaleDPI();
        const Vector2 mouse_world_pos = GetScreenToWorld2D(GetMousePosition() * dpi, camera);

        DrawTextEx(GetFontDefault(), TextFormat("[%.2f, %.2f]", mouse_world_pos.x, mouse_world_pos.y),
                   Vector2Add(GetMousePosition(), (Vector2){-44, -24}), 20, 2, BLACK);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    CloseWindow(); // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}
