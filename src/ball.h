//
// Created by rahul on 17-06-2025.
//

#ifndef BALL_H
#define BALL_H

#pragma once

class Ball {
public:
    float x, y;
    float speed_x, speed_y;
    int radius;

    Ball(float x, float y, float speed_x, float speed_y, int radius);

    void draw() const;
};


#endif //BALL_H
