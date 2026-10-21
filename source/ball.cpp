#include "ball.hpp"

#include "log.hpp"

namespace pong
{
    void Ball::update(float delta_time)
    {
        const auto& stat = stats();
        y += delta_time * vy * stat[Stat::Speed];
        x += delta_time * vx * stat[Stat::Speed];
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
