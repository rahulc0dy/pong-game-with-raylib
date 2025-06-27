
#include <complex>
#include <raylib.h>

#include "ball.h"
#include "paddle.h"

const unsigned int winScore = 10;

const Color background = Color{18, 26, 28, 255};
const Color accent = Color{3, 76, 83, 255};

const Color playerColor = Color{146, 255, 236, 255};
const Color computerColor = Color{234, 134, 161, 255};

enum GameState {
    MENU,
    PLAYING,
    GAME_OVER
};

int main() {
    int screenWidth = 1200;
    int screenHeight = 800;

    const int ballRadius = 10;
    const float ballSpeed = 400.0f;
    const float paddleSpeed = 600.0f;
    const int paddleWidth = 20;
    const int paddleHeight = 100;
    const int topOffset = 80;

    GameState state = MENU;

    InitAudioDevice();
    InitWindow(screenWidth, screenHeight, "Pong");

    SetTraceLogLevel(LOG_NONE);
    SetTraceLogLevel(LOG_NONE);

    Image ballImage = LoadImage("assets/ball.png");
    Image paddle1 = LoadImage("assets/paddle1.png");
    Image paddle2 = LoadImage("assets/paddle2.png");
    Image icon = LoadImage("assets/icon.png");
    Sound ballHit = LoadSound("assets/bounce.wav");
    Sound scoreSound = LoadSound("assets/point.wav");
    Music backgroundMusic = LoadMusicStream("assets/background-music.mp3");

    ImageResize(&paddle1, paddleWidth, paddleHeight);
    ImageResize(&paddle2, paddleWidth, paddleHeight);
    ImageResize(&ballImage, ballRadius * 2, ballRadius * 2);
    SetSoundVolume(ballHit, 1.0f);
    SetSoundVolume(scoreSound, 1.0f);

    const Texture2D ballTexture = LoadTextureFromImage(ballImage);
    const Texture2D paddleTexture1 = LoadTextureFromImage(paddle1);
    const Texture2D paddleTexture2 = LoadTextureFromImage(paddle2);

    SetWindowState(FLAG_WINDOW_RESIZABLE);
    SetWindowIcon(icon);
    SetTargetFPS(1000);

    Ball ball(
        screenWidth / 2,
        screenHeight / 2,
        ballSpeed,
        ballSpeed,
        ballRadius,
        ballTexture,
        topOffset,
        scoreSound
    );

    Paddle player(
        10,
        screenHeight / 4,
        paddleWidth,
        paddleHeight,
        paddleSpeed,
        paddleTexture1,
        topOffset
    );
    ComputerPaddle computer(
        screenWidth - 30,
        10,
        paddleWidth,
        paddleHeight,
        paddleSpeed / 1.5f,
        paddleTexture2
    );


    SetMusicVolume(backgroundMusic, 1.0f);
    PlayMusicStream(backgroundMusic);
    while (!WindowShouldClose()) {
        const float delta = GetFrameTime();
        screenWidth = GetScreenWidth();
        screenHeight = GetScreenHeight();
        const int fps = GetFPS();


        // 1. Event Handling
        if (IsKeyPressed(KEY_ESCAPE)) {
            CloseWindow();
        }

        UpdateMusicStream(backgroundMusic);

        BeginDrawing();
        ClearBackground(background);

        switch (state) {
            case MENU:
                DrawText("PONG", screenWidth / 2 - 100, screenHeight / 2 - 60, 64, ORANGE);
                DrawText("Press ENTER to Play", screenWidth / 2 - 180, screenHeight / 2 + 20, 32, WHITE);
                DrawText("Game ends when score difference becomes 10", screenWidth / 2 - 360, screenHeight / 2 + 80, 32,
                         SKYBLUE);
                ball.cpuScore = ball.playerScore = 0;
                if (IsKeyPressed(KEY_ENTER)) state = PLAYING;
                break;
            case PLAYING: {
                ball.update(delta);
                player.update(delta);
                computer.update(delta, ball.y);

                if (CheckCollisionCircleRec(Vector2{ball.x, ball.y}, ball.radius,
                                            Rectangle{player.x, player.y, player.width, player.height})) {
                    ball.speed_x *= -1; // Reverse direction on x-axis
                    ball.x = player.x + player.width + ball.radius; // Move ball out of paddle
                    PlaySound(ballHit);
                }

                if (CheckCollisionCircleRec(Vector2{ball.x, ball.y}, ball.radius,
                                            Rectangle{computer.x, computer.y, computer.width, computer.height})) {
                    ball.speed_x *= -1; // Reverse direction on x-axis
                    ball.x = computer.x - ball.radius; // Move ball out of paddle
                    PlaySound(ballHit);
                }

                if (ball.x <= 0 || ball.x >= screenWidth) {
                    PlaySound(scoreSound);
                }

                computer.x = screenWidth - 30;

                // 3. Drawing


                // Court
                DrawLine(
                    screenWidth / 2,
                    0,
                    screenWidth / 2,
                    screenHeight,
                    Color{184, 207, 206, 150}
                );
                DrawRectangle(0, 0, screenWidth, topOffset, accent);

                // Ball and Paddles
                ball.draw();
                player.draw();
                computer.draw();

                // scores
                DrawText(
                    TextFormat("%i", ball.cpuScore),
                    screenWidth / 4,
                    20,
                    32,
                    playerColor
                );
                DrawText(
                    TextFormat("%i", ball.playerScore),
                    3 * screenWidth / 4,
                    20,
                    32,
                    computerColor);
                // FPS Counter
                DrawFPS(GetScreenWidth() - 100, 10);

                if (std::abs(static_cast<int>(ball.playerScore - ball.cpuScore)) >= winScore) {
                    state = GAME_OVER;
                }

                break;
            }
            case GAME_OVER: {
                DrawText("GAME OVER", screenWidth / 2 - 150, screenHeight / 2 - 60, 64, RED);
                DrawText("Press R to Restart", screenWidth / 2 - 120, screenHeight / 2 + 20, 32, WHITE);
                if (IsKeyPressed(KEY_R)) state = MENU;
                break;
            }
        }

        EndDrawing();
    }

    StopMusicStream(backgroundMusic);
    UnloadTexture(paddleTexture1);
    UnloadTexture(paddleTexture2);
    UnloadTexture(ballTexture);
    CloseWindow();
    return 0;
}
