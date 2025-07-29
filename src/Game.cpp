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

    if (!m_net.init()) {
        m_networkConnected = false;
    }
}

Game::~Game() {
    StopMusicStream(m_backgroundMusic);
    UnloadTexture(m_paddleTexture1.value());
    UnloadTexture(m_paddleTexture2.value());
    UnloadTexture(m_ballTexture.value());
    CloseWindow();
}

bool Game::initNetwork(const char *host, uint16_t port) {
    if (!m_net.init()) {
        m_networkConnected = false;
    }
    m_networkConnected = m_net.connect(host, port);
    return m_networkConnected;
}

void Game::disconnectNetwork() {
    m_net.disconnect();
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
        m_paddleTexture2.value()
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
        if (IsKeyPressed(KEY_ESCAPE)) {
            CloseWindow();
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
                    if (m_net.init() && m_net.listen(12345)) {
                        m_gameMode = OnlineHost;
                        m_gameState = Playing;
                    }
                }
                y += btnH + 10;
                if (GuiButton(Rectangle(x, y, btnW, btnH), "Online LAN (Join)")) {
                    if (m_net.init() && m_net.connect(ipBuffer, 12345)) {
                        m_gameMode = OnlineClient;
                        m_gameState = Playing;
                    }
                }
                DrawText("Use ESC at any time to return", 20, 20, 20, LIGHTGRAY);
                break;
            }

            case Playing: {
                switch (m_gameMode) {
                    case Computer:
                        player1.updateWASD(delta);
                        player1.updateArrows(delta);
                        player2.autoUpdate(delta, ball.y);
                        break;
                    case Local:
                        player1.updateWASD(delta);
                        player2.updateArrows(delta);
                        break;
                    case OnlineHost:
                        player1.updateWASD(delta);
                        receivePaddleInput(player2); // remote player1
                        ball.update(delta);
                        sendGameState(ball, player1, player2);
                        break;
                    case OnlineClient:
                        player1.updateArrows(delta);
                        sendPaddleInput(player1);
                        receiveGameState(ball, player2, player1); // swap paddles
                        break;
                }

                ball.update(delta);

                // Collision and scoring logic (shared)
                if (CheckCollisionCircleRec(Vector2{ball.x, ball.y}, ball.radius,
                                            Rectangle{player1.x, player1.y, player1.width, player1.height})) {
                    ball.speed_x *= -1;
                    ball.x = player1.x + player1.width + ball.radius;
                    PlaySound(m_ballHit);
                }
                if (CheckCollisionCircleRec(Vector2{ball.x, ball.y}, ball.radius,
                                            Rectangle{player2.x, player2.y, player2.width, player2.height})) {
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
                    ball.resetBall();
                }
                break;
            }
        }

        EndDrawing();
    }
}

void Game::update() {
}

void Game::sendGameState(const Ball &ball, const Paddle &player1, const Paddle &player2) {
    GamePacket pkt{ball.x, ball.y, ball.speed_x, ball.speed_y, player1.y, player2.y, ball.playerScore, ball.cpuScore};
    m_net.service(0, [](const Net::Event &) {
    });
    m_net.send(reinterpret_cast<const uint8_t *>(&pkt), sizeof(pkt));
}

void Game::receiveGameState(Ball &ball, Paddle &player1, Paddle &player2) {
    m_net.service(0, [&](const Net::Event &evt) {
        if (evt.type == Net::EventType::Receive && evt.dataLength == sizeof(GamePacket)) {
            const GamePacket *pkt = reinterpret_cast<const GamePacket *>(evt.data);
            ball.x = pkt->ballX;
            ball.y = pkt->ballY;
            ball.speed_x = pkt->ballSpeedX;
            ball.speed_y = pkt->ballSpeedY;
            player1.y = pkt->playerY;
            player2.y = pkt->computerY;
            ball.playerScore = pkt->playerScore;
            ball.cpuScore = pkt->cpuScore;
        }
    });
}

void Game::sendPaddleInput(const Paddle &player1) {
    float y = player1.y;
    m_net.send(reinterpret_cast<const uint8_t *>(&y), sizeof(y));
}

void Game::receivePaddleInput(Paddle &player1) {
    m_net.service(0, [&](const Net::Event &evt) {
        if (evt.type == Net::EventType::Receive && evt.dataLength == sizeof(float)) {
            player1.y = *reinterpret_cast<const float *>(evt.data);
        }
    });
}
