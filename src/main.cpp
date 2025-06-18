#include <functional>
#include <raylib.h>
#include <string>

#include "ball.h"
#include "paddle.h"


const Color background = Color{18, 26, 28, 255};
const Color accent = Color{3, 76, 83, 255};

const Color playerColor = Color{146, 255, 236, 255};
const Color computerColor = Color{234, 134, 161, 255};

int main() {
    int screenWidth = 1200;
    int screenHeight = 800;

    const int ballRadius = 10;
    const float ballSpeed = 400.0f;
    const float paddleSpeed = 600.0f;
    const int paddleWidth = 20;
    const int paddleHeight = 100;
    const int topOffset = 80;


    InitWindow(screenWidth, screenHeight, "Pong");

    Image ballImage = LoadImage("D:/Programming/CPP/Flappy Bird/assets/ball.png");
    Image paddle1 = LoadImage("D:/Programming/CPP/Flappy Bird/assets/paddle1.png");
    Image paddle2 = LoadImage("D:/Programming/CPP/Flappy Bird/assets/paddle2.png");

    ImageResize(&paddle1, paddleWidth, paddleHeight);
    ImageResize(&paddle2, paddleWidth, paddleHeight);
    ImageResize(&ballImage, ballRadius * 2, ballRadius * 2);

    const Texture2D ballTexture = LoadTextureFromImage(ballImage);
    const Texture2D paddleTexture1 = LoadTextureFromImage(paddle1);
    const Texture2D paddleTexture2 = LoadTextureFromImage(paddle2);

    SetWindowState(FLAG_WINDOW_RESIZABLE);
    SetWindowIcon(ballImage);
    SetTargetFPS(1000);

    Ball ball(screenWidth / 2, screenHeight / 2, ballSpeed, ballSpeed, ballRadius, ballTexture, topOffset);

    Paddle player(10, screenHeight / 4, paddleWidth, paddleHeight, paddleSpeed, paddleTexture1, topOffset);
    ComputerPaddle computer(screenWidth - 30, 10, paddleWidth, paddleHeight, paddleSpeed / 1.5f, paddleTexture2);


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
        ClearBackground(background);

        // Court
        DrawLine(screenWidth / 2, 0, screenWidth / 2, screenHeight, Color{184, 207, 206, 150});
        DrawRectangle(0, 0, screenWidth, topOffset, accent);

        // Ball and Paddles
        ball.draw();
        player.draw();
        computer.draw();

        // scores
        DrawText(TextFormat("%i", ball.cpuScore), screenWidth / 4, 20, 32, playerColor);
        DrawText(TextFormat("%i", ball.playerScore), 3 * screenWidth / 4, 20, 32, computerColor);

        // FPS Counter
        DrawFPS(GetScreenWidth() - 100, 10);

        EndDrawing();
    }

    UnloadImage(ballImage);
    UnloadTexture(ballTexture);
    CloseWindow();
    return 0;
}
