#include <print>
#include "raylib.h"
#include "Window.h"

int main() {
    std::println("Hello World!");

    auto window = Window("Routing Simulation");
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_MSAA_4X_HINT);

    InitWindow(window.width, window.height, window.title.c_str());

    SetTargetFPS(60);               // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------


    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {

        // Update
        if (IsWindowResized()) {

            window.width = GetRenderWidth();
            window.height = GetRenderHeight();
        }

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

        ClearBackground(RAYWHITE);

        // DrawText("Congrats! You created your first window!", 190, 200, 20, DARKGRAY);

        // DrawLineEx(Vector2 {20, 40 }, Vector2 {400, 30}, 7., DARKBLUE);

        DrawCircle(window.width/2, window.height/2, 30, PURPLE);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}
