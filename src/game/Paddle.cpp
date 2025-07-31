//
// Created by rahul on 17-06-2025.
//

#include "Paddle.hpp"

#include "raylib.h"

Paddle::Paddle(float x, float y, float width, float height, int speed, Texture2D texture, int topOffset)
    : x(x), y(y), width(width), height(height), speed(speed), texture(texture), topOffset(topOffset) {
}

void Paddle::draw() const {
    // DrawRectangle(x, y, width, height, WHITE);
    DrawTexture(texture, x, y, WHITE);
}

void Paddle::autoUpdate(float delta, int ballY) {
    if (y + height / 2 < ballY) {
        y += speed * delta; // Move down
    } else if (y + height / 2 > ballY) {
        y -= speed * delta; // Move up
    }

    if (y < topOffset) {
        y = topOffset;
    }
    if (y + height > GetScreenHeight()) {
        y = GetScreenHeight() - height;
    }
}

void Paddle::updateWASD(float delta) {
    if (IsKeyDown(KEY_W)) y -= speed * delta;
    if (IsKeyDown(KEY_S)) y += speed * delta;
    if (y < topOffset) y = topOffset;
    if (y + height > GetScreenHeight()) y = GetScreenHeight() - height;
}

void Paddle::updateArrows(float delta) {
    if (IsKeyDown(KEY_UP)) y -= speed * delta;
    if (IsKeyDown(KEY_DOWN)) y += speed * delta;
    if (y < topOffset) y = topOffset;
    if (y + height > GetScreenHeight()) y = GetScreenHeight() - height;
}
