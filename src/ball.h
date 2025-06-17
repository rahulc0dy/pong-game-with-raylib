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
    Color ballColor = ORANGE;

    Ball(float x, float y, float speed_x, float speed_y, int radius, Color color = ORANGE);

    void update(float delta);

    void draw() const;

    void resetBall();
};


#endif //BALL_H
