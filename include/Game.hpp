#pragma once
#include <optional>
#include <raylib.h>

#include "Ball.hpp"
#include "Enet.hpp"
#include "Paddle.hpp"


struct GamePacket {
    float ballX, ballY, ballSpeedX, ballSpeedY;
    float playerY, computerY;
    uint32_t playerScore, cpuScore;
};

enum GameState {
    Menu,
    Playing,
    GameOver
};

enum GameMode {
    Computer,
    Local,
    OnlineHost,
    OnlineClient
};


class Game {
public:
    Game();

    ~Game();

    void start();

private:
    bool initNetwork(const char *host, uint16_t port);

    void disconnectNetwork();

    void update();

    void sendGameState(const Ball &ball, const Paddle &player1, const Paddle &player2);

    void receiveGameState(Ball &ball, Paddle &player1, Paddle &player2);

    void sendPaddleInput(const Paddle &player);

    void receivePaddleInput(Paddle &player);

private:
    int m_screenWidth = 1200;
    int m_screenHeight = 800;

    const int m_ballRadius = 10;
    const float m_ballSpeed = 400.0f;
    const float m_paddleSpeed = 600.0f;
    const int m_paddleWidth = 20;
    const int m_paddleHeight = 100;
    const int m_topOffset = 80;

    const unsigned int winScore = 10;

    const Color m_backgroundColor = Color{18, 26, 28, 255};
    const Color m_accentColor = Color{3, 76, 83, 255};

    const Color m_playerColor = Color{146, 255, 236, 255};
    const Color m_computerColor = Color{234, 134, 161, 255};

    Image m_icon;
    Sound m_ballHit;
    Sound m_scoreSound;
    Music m_backgroundMusic;

    std::optional<Texture2D> m_ballTexture;
    std::optional<Texture2D> m_paddleTexture1;
    std::optional<Texture2D> m_paddleTexture2;

    GameState m_gameState = Menu;
    GameMode m_gameMode = Computer;

    Net::Wrapper m_net;
    bool m_networkConnected = false;
};
