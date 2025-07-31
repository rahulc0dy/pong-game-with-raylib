#include "Game.hpp"

#include "Ball.hpp"
#include "Paddle.hpp"

#define RAYGUI_IMPLEMENTATION
#include <raygui.h>


Game::Game() {
    InitAudioDevice();
    InitWindow(m_screenWidth, m_screenHeight, "Ping Pong");

    SetTraceLogLevel(LOG_NONE);

    Image ballImage = LoadImage("assets/ball.png");
    Image paddle1 = LoadImage("assets/paddle1.png");
    Image paddle2 = LoadImage("assets/paddle2.png");

    m_icon = LoadImage("assets/icon.png");
    m_ballHit = LoadSound("assets/bounce.wav");
    m_scoreSound = LoadSound("assets/point.wav");
    m_backgroundMusic = LoadMusicStream("assets/background-music.mp3");

    ImageResize(&paddle1, m_paddleWidth, m_paddleHeight);
    ImageResize(&paddle2, m_paddleWidth, m_paddleHeight);
    ImageResize(&ballImage, m_ballRadius * 2, m_ballRadius * 2);
    SetSoundVolume(m_ballHit, 1.0f);
    SetSoundVolume(m_scoreSound, 1.0f);

    m_ballTexture = LoadTextureFromImage(ballImage);
    m_paddleTexture1 = LoadTextureFromImage(paddle1);
    m_paddleTexture2 = LoadTextureFromImage(paddle2);

    SetWindowState(FLAG_WINDOW_RESIZABLE);
    SetWindowIcon(m_icon);
    SetTargetFPS(1000);
}

Game::~Game() {
    StopMusicStream(m_backgroundMusic);
    UnloadTexture(m_paddleTexture1.value());
    UnloadTexture(m_paddleTexture2.value());
    UnloadTexture(m_ballTexture.value());
    CloseWindow();
    CloseAudioDevice();
}

void Game::start() {
    Ball ball(
        m_screenWidth / 2,
        m_screenHeight / 2,
        m_ballSpeed,
        m_ballSpeed,
        m_ballRadius,
        m_ballTexture.value(),
        m_topOffset,
        m_scoreSound
    );

    Paddle player1(
        10,
        m_screenHeight / 4,
        m_paddleWidth,
        m_paddleHeight,
        m_paddleSpeed,
        m_paddleTexture1.value(),
        m_topOffset
    );
    Paddle player2(
        m_screenWidth - 30,
        m_screenHeight / 3,
        m_paddleWidth,
        m_paddleHeight,
        m_paddleSpeed,
        m_paddleTexture2.value(),
        m_topOffset
    );

    static char ipBuffer[32] = "127.0.0.1";
    static bool editIp = false;

    SetMusicVolume(m_backgroundMusic, 1.0f);
    PlayMusicStream(m_backgroundMusic);


    while (!WindowShouldClose()) {
        const float delta = GetFrameTime();
        m_screenWidth = GetScreenWidth();
        m_screenHeight = GetScreenHeight();
        const int fps = GetFPS();


        // 1. Event Handling
        if (IsKeyPressed(KEY_ENTER)) {
            m_gameState = Menu;
            ball.resetBall();
            ball.resetScores();
        }

        UpdateMusicStream(m_backgroundMusic);

        BeginDrawing();
        ClearBackground(m_backgroundColor);

        switch (m_gameState) {
            case Menu: {
                constexpr float btnW = 300.0f;
                constexpr float btnH = 50.0f;
                const float x = m_screenWidth / 2 - btnW / 2;
                float y = m_screenHeight / 2 - 2 * (btnH + 10);
                if (GuiButton(Rectangle(x, y, btnW, btnH), "Vs Computer")) {
                    m_gameMode = Computer;
                    m_gameState = Playing;
                }
                y += btnH + 10;
                if (GuiButton(Rectangle(x, y, btnW, btnH), "Local: WASD + Arrows")) {
                    m_gameMode = Local;
                    m_gameState = Playing;
                }
                y += btnH + 10;

                if (GuiButton(Rectangle(x, y, btnW, btnH), "Online LAN (Host)")) {
                    if (m_networkManager.init()) {
                        std::string roomCode;
                        if (m_networkManager.hostGame(roomCode)) {
                            m_currentRoomCode = roomCode;
                            m_gameMode = OnlineHost;
                            m_gameState = WaitingForPlayer;
                        }
                    }
                }
                y += btnH + 10;

                if (GuiButton(Rectangle(x, y, btnW, btnH), "Online LAN (Join)")) {
                    m_showRoomCodeInput = true;
                }
                if (m_showRoomCodeInput) {
                    DrawRectangle(m_screenWidth / 2 - 150, m_screenHeight / 2 - 100, 300, 200, Color{0, 0, 0, 200});
                    DrawRectangleLines(m_screenWidth / 2 - 150, m_screenHeight / 2 - 100, 300, 200, WHITE);
                    DrawText("Enter Room Code", m_screenWidth / 2 - 100, m_screenHeight / 2 - 80, 20, WHITE);

                    // Room code input box
                    GuiTextBox(Rectangle{m_screenWidth / 2 - 100, m_screenHeight / 2 - 40, 200, 40},
                               m_roomCodeInput, 6, true);

                    if (GuiButton(Rectangle{m_screenWidth / 2 - 100, m_screenHeight / 2 + 20, 90, 40}, "Join")) {
                        if (m_networkManager.init() && m_networkManager.joinGame(m_roomCodeInput)) {
                            m_gameMode = OnlineClient;
                            m_gameState = WaitingForPlayer;
                            m_showRoomCodeInput = false;
                        }
                    }

                    if (GuiButton(Rectangle{m_screenWidth / 2 + 10, m_screenHeight / 2 + 20, 90, 40}, "Cancel")) {
                        m_showRoomCodeInput = false;
                    }
                }

                DrawText("Use ESC at any time to return", 20, 20, 20, LIGHTGRAY);
                break;
            }

            case WaitingForPlayer: {
                DrawText("Waiting for player...", m_screenWidth / 2 - 150, m_screenHeight / 2 - 40, 30, WHITE);

                if (m_gameMode == OnlineHost) {
                    DrawText("Room Code:", m_screenWidth / 2 - 150, m_screenHeight / 2 + 20, 20, WHITE);
                    DrawText(m_currentRoomCode.c_str(), m_screenWidth / 2 + 50, m_screenHeight / 2 + 20, 30, YELLOW);
                }

                // Update network and check for connection
                m_networkManager.update();
                if (m_networkManager.isConnected()) {
                    m_gameState = Playing;
                }

                if (IsKeyPressed(KEY_ESCAPE)) {
                    m_networkManager.disconnect();
                    m_gameState = Menu;
                }
                break;
            }

            case Playing: {
                switch (m_gameMode) {
                    case Computer:
                        player1.updateWASD(delta);
                        player1.updateArrows(delta);
                        player2.autoUpdate(delta, ball.y);
                        ball.update(delta);
                        break;
                    case Local:
                        player1.updateWASD(delta);
                        player2.updateArrows(delta);
                        ball.update(delta);
                        break;
                    case OnlineHost:
                        player1.updateWASD(delta);
                        m_networkManager.getPaddleInput(player2);
                        ball.update(delta);
                        m_networkManager.sendGameState(ball, player1, player2);
                        break;
                    case OnlineClient:
                        player2.updateArrows(delta);
                        m_networkManager.sendPaddleInput(player2);
                        m_networkManager.getGameState(ball, player1, player2);
                        break;
                }

                m_networkManager.update();

                if (CheckCollisionCircleRec(Vector2{ball.x, ball.y}, ball.radius,
                                            Rectangle(player1.x, player1.y, player1.width, player1.height))) {
                    ball.speed_x *= -1;
                    ball.x = player1.x + player1.width + ball.radius;
                    PlaySound(m_ballHit);
                }
                if (CheckCollisionCircleRec(Vector2{ball.x, ball.y}, ball.radius,
                                            Rectangle(player2.x, player2.y, player2.width, player2.height))) {
                    ball.speed_x *= -1;
                    ball.x = player2.x - ball.radius;
                    PlaySound(m_ballHit);
                }
                if (ball.x <= 0 || ball.x >= m_screenWidth) {
                    PlaySound(m_scoreSound);
                }
                player2.x = m_screenWidth - 30;

                // Drawing
                DrawLine(m_screenWidth / 2, 0, m_screenWidth / 2, m_screenHeight, Color{184, 207, 206, 150});
                DrawRectangle(0, 0, m_screenWidth, m_topOffset, m_accentColor);
                ball.draw();
                player1.draw();
                player2.draw();
                DrawText(TextFormat("%i", ball.cpuScore), m_screenWidth / 4, 20, 32, m_playerColor);
                DrawText(TextFormat("%i", ball.playerScore), 3 * m_screenWidth / 4, 20, 32, m_computerColor);
                DrawFPS(GetScreenWidth() - 100, 10);

                if (ball.cpuScore >= winScore || ball.playerScore >= winScore) {
                    m_gameState = GameOver;
                }
                break;
            }
            case GameOver: {
                DrawText("GAME OVER", m_screenWidth / 2 - 150, m_screenHeight / 2 - 60, 64, RED);
                DrawText("Press R to Restart", m_screenWidth / 2 - 120, m_screenHeight / 2 + 20, 32, WHITE);
                if (IsKeyPressed(KEY_R)) {
                    m_gameState = Menu;
                    ball.resetScores();
                    ball.resetBall();
                }
                break;
            }
        }

        EndDrawing();
    }
}
