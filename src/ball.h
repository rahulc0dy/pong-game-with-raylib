//
// Created by rahul on 17-06-2025.
//

#ifndef BALL_H
#define BALL_H

#pragma once
#include "raylib.h"

class Ball {
public:
    float x, y;
    float speed_x, speed_y;
    int radius;
    unsigned playerScore = 0;
    unsigned cpuScore = 0;
    Texture2D texture;
    int topOffset = 0;

    Ball(float x, float y, float speed_x, float speed_y, int radius, Texture2D texture, int topOffset = 0);

    void update(float delta);

    void draw() const;

    void resetBall();
};


#endif //BALL_H
