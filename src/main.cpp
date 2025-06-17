#include <functional>
#include <raylib.h>
#include <string>

#include "ball.h"
#include "paddle.h"

const Color darkGreen = Color{20, 160, 133, 255};
const Color green = Color{38, 185, 154, 255};
const Color lightGreen = Color{129, 204, 184, 255};
const Color yellow = Color{243, 213, 91, 255};

int main() {
    int screenWidth = 1200;
    int screenHeight = 800;

    const int ballRadius = 10;
    const float ballSpeed = 300.0f;
    const float paddleSpeed = 400.0f;

    InitWindow(screenWidth, screenHeight, "Pong");
    SetWindowState(FLAG_WINDOW_RESIZABLE);
    SetTargetFPS(120);

    Ball ball(screenWidth / 2, screenHeight / 2, ballSpeed, ballSpeed, ballRadius, yellow);

    Paddle player(10, 10, 20, 100, paddleSpeed);
    ComputerPaddle computer(screenWidth - 30, 10, 20, 100, paddleSpeed / 1.5f);


    while (!WindowShouldClose()) {
        const float delta = GetFrameTime();
        screenWidth = GetScreenWidth();
        screenHeight = GetScreenHeight();
        const int fps = GetFPS();


        // 1. Event Handling
        if (IsKeyPressed(KEY_ESCAPE)) {
            CloseWindow();
        }

        // 2. Updating Positions
        ball.update(delta);
        player.update(delta);
        computer.update(delta, ball.y);

        if (CheckCollisionCircleRec(Vector2{ball.x, ball.y}, ball.radius,
                                    Rectangle{player.x, player.y, player.width, player.height})) {
            ball.speed_x *= -1; // Reverse direction on x-axis
            ball.x = player.x + player.width + ball.radius; // Move ball out of paddle
        }

        if (CheckCollisionCircleRec(Vector2{ball.x, ball.y}, ball.radius,
                                    Rectangle{computer.x, computer.y, computer.width, computer.height})) {
            ball.speed_x *= -1; // Reverse direction on x-axis
            ball.x = computer.x - ball.radius; // Move ball out of paddle
        }
        computer.x = screenWidth - 30;

        // 3. Drawing
        BeginDrawing();
        ClearBackground(darkGreen);

        // Court
        DrawRectangle(screenWidth / 2, 0, screenWidth / 2, screenHeight, lightGreen);
        DrawLine(screenWidth / 2, 0, screenWidth / 2, screenHeight, WHITE);
        DrawCircle(screenWidth / 2, screenHeight / 2, 150, green);

        // Ball and Paddles
        ball.draw();
        player.draw();
        computer.draw();

        // scores
        DrawText(TextFormat("%i", ball.playerScore), screenWidth / 4, 20, 32, WHITE);
        DrawText(TextFormat("%i", ball.cpuScore), 3 * screenWidth / 4, 20, 32, WHITE);

        // FPS Counter
        DrawText(TextFormat("FPS: %i", fps), GetScreenWidth() - 100, 10, 16, WHITE);

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
