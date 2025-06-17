//
// Created by rahul on 17-06-2025.
//

#ifndef PADDLE_H
#define PADDLE_H


class Paddle {
public:
    float x, y, width, height;
    int speed;

    Paddle(float x, float y, float width, float height, int speed);

    void draw() const;

    void update(float delta);
};

class ComputerPaddle : public Paddle {
public:
    ComputerPaddle(float x, float y, float width, float height, int speed);

    void update(float delta, int ballY);
};


#endif //PADDLE_H
