//
// Created by rahul on 17-06-2025.
//

#include "ball.h"
#include <raylib.h>

Ball::Ball(float x, float y, float speed_x, float speed_y, int radius)
    : x(x), y(y), speed_x(speed_x), speed_y(speed_y), radius(radius) {
}

void Ball::draw() const {
    DrawCircle(x, y, radius, WHITE);
}


