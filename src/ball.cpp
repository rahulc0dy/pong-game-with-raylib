//
// Created by rahul on 17-06-2025.
//

#include "ball.h"
#include <raylib.h>

Ball::Ball(float x, float y, float speed_x, float speed_y, int radius, Color color)
    : x(x), y(y), speed_x(speed_x), speed_y(speed_y), radius(radius), ballColor(color) {
}

void Ball::draw() const {
    DrawCircle(x, y, radius, ORANGE);
}

void Ball::update(const float delta) {
    x += speed_x * delta;
    y += speed_y * delta;
    // Bounce off walls
    if (x - radius < 0 || x + radius > GetScreenWidth()) {
        speed_x *= -1; // Reverse direction on x-axis
    }
    if (y - radius < 0 || y + radius > GetScreenHeight()) {
        speed_y *= -1; // Reverse direction on y-axis
    }

    if (x + radius >= GetScreenWidth()) {
        cpuScore++;
        resetBall();
    }

    if (x - radius <= 0) {
        playerScore++;
        resetBall();
    }
}

void Ball::resetBall() {
    x = GetScreenWidth() / 2;
    y = GetScreenHeight() / 2;

    int directions[] = {-1, 1};
    speed_x *= directions[GetRandomValue(0, 1)];
    speed_y *= directions[GetRandomValue(0, 1)];
}
