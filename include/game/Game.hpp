#pragma once
#include <optional>
#include <raylib.h>

#include "Ball.hpp"
#include "Enet.hpp"
#include "NetworkManager.hpp"
#include "Paddle.hpp"

enum GameState {
    Menu,
    WaitingForPlayer,
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

    NetworkManager m_networkManager;
    bool m_showRoomCodeInput = false;
    char m_roomCodeInput[6] = {0};
    std::string m_currentRoomCode;
};
