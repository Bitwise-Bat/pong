#include "ball.hpp"

namespace pong
{
    void Ball::update(float delta_time)
    {
        rect.y += delta_time * vy * speed;
        rect.x += delta_time * vx * speed;
    }

    void Ball::bounce_x()
    {
        vx = -vx;
    }

    void Ball::bounce_y()
    {
        vy = -vy;
    }
}
