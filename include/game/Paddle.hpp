//
// Created by rahul on 17-06-2025.
//
#pragma once

#include "raylib.h"


class Paddle {
public:
    float x, y, width, height;
    int speed;
    Texture2D texture;
    int topOffset;

    Paddle(float x, float y, float width, float height, int speed, Texture2D texture, int topOffset = 0);

    void draw() const;

    void updateWASD(float delta);

    void updateArrows(float delta);

    void autoUpdate(float delta, int ballY);

private:
};

class ComputerPaddle : public Paddle {
public:
    ComputerPaddle(float x, float y, float width, float height, int speed, Texture2D texture);

    void update(float delta, int ballY);
};

