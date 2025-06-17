#include <raylib.h>
#include <string>

#include "ball.h"

Color darkGreen = Color{20, 160, 133, 255};


int main() {
    const int screenWidth = 800;
    const int screenHeight = 600;

    const int ballRadius = 10;
    const float ballSpeed = 100.0f;

    InitWindow(screenWidth, screenHeight, "Pong");
    SetWindowState(FLAG_WINDOW_RESIZABLE);
    SetTargetFPS(120);

    Ball ball(screenWidth / 2, screenHeight / 2, ballSpeed, ballSpeed, ballRadius);


    while (!WindowShouldClose()) {
        const int screenWidth = GetScreenWidth();
        const int screenHeight = GetScreenHeight();
        const float delta = GetFrameTime();

        // Frame Rate and Timing
        const int fps = GetFPS();
        std::string fpsDisplay = "FPS: " + std::to_string(fps);
        DrawText(fpsDisplay.c_str(), 10, 10, 20, WHITE);

        // 1. Event Handling
        if (IsKeyPressed(KEY_ESCAPE)) {
            CloseWindow();
        }

        // 2. Updating Positions
        ball.x += ball.speed_x * delta;
        ball.y += ball.speed_y * delta;
        // Bounce off walls
        if (ball.x - ball.radius < 0 || ball.x + ball.radius > screenWidth) {
            ball.speed_x *= -1; // Reverse direction on x-axis
        }
        if (ball.y - ball.radius < 0 || ball.y + ball.radius > screenHeight) {
            ball.speed_y *= -1; // Reverse direction on y-axis
        }

        // 3. Drawing
        BeginDrawing();
        ClearBackground(darkGreen);
        ball.draw();
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
