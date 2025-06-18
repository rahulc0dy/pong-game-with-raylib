//
// Created by rahul on 17-06-2025.
//

#include "paddle.h"

#include "raylib.h"

Paddle::Paddle(float x, float y, float width, float height, int speed, Texture2D texture, int topOffset)
    : x(x), y(y), width(width), height(height), speed(speed), texture(texture), topOffset(topOffset) {
}

void Paddle::draw() const {
    // DrawRectangle(x, y, width, height, WHITE);
    DrawTexture(texture, x, y, WHITE);
}

void Paddle::update(float delta) {
    if (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP)) {
        y -= speed * delta; // Move up
    }
    if (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN)) {
        y += speed * delta; // Move down
    }

    // Keep paddle within screen bounds
    if (y < topOffset) {
        y = topOffset;
    }
    if (y + height > GetScreenHeight()) {
        y = GetScreenHeight() - height;
    }
}

ComputerPaddle::ComputerPaddle(float x, float y, float width, float height, int speed, Texture2D texture)
    : Paddle(x, y, width, height, speed, texture) {
}

void ComputerPaddle::update(float delta, int ballY) {
    // Simple AI to follow the ball
    if (y + height / 2 < ballY) {
        y += speed * delta; // Move down
    } else if (y + height / 2 > ballY) {
        y -= speed * delta; // Move up
    }

    // Keep paddle within screen bounds
    if (y < topOffset) {
        y = 0;
    }
    if (y + height > GetScreenHeight()) {
        y = GetScreenHeight() - height;
    }
}
