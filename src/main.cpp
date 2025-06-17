#include <raylib.h>
#include <string>


int main() {
    Color darkGreen = Color{20, 160, 133, 255};

    const int screenWidth = 800;
    const int screenHeight = 600;

    InitWindow(screenWidth, screenHeight, "Pong");
    SetWindowState(FLAG_WINDOW_RESIZABLE);

    float ballX = 400.0f;
    float ballY = 300.0f;
    float ballSize = 10.0f;
    float ballSpeed = 2.0f;


    while (!WindowShouldClose()) {
        // Frame Rate and Timing
        const float delta = GetFrameTime() * 1000;
        const int fps = GetFPS();
        std::string fpsDisplay = "FPS: " + std::to_string(fps);
        DrawText(fpsDisplay.c_str(), 10, 10, 20, WHITE);

        // 1. Event Handling
        if (IsKeyPressed(KEY_ESCAPE)) {
            CloseWindow();
        }

        // 2. Updating Positions
        if (IsKeyDown(KEY_RIGHT) and ballX < screenWidth - ballSize) {
            ballX += ballSpeed * delta; // Move right
        }
        if (IsKeyDown(KEY_LEFT) and ballX > ballSize) {
            ballX -= ballSpeed * delta; // Move left
        }
        if (IsKeyDown(KEY_UP) and ballY > ballSize) {
            ballY -= ballSpeed * delta; // Move up
        }
        if (IsKeyDown(KEY_DOWN) and ballY < screenHeight - ballSize) {
            ballY += ballSpeed * delta; // Move down
        }

        // 3. Drawing
        BeginDrawing();
        ClearBackground(darkGreen);
        DrawCircle(ballX, ballY, ballSize, WHITE);
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
