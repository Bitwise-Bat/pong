#pragma once

#include "math.hpp"

namespace pong
{
    class Ball
    {
    public:
        Rect rect{};
        float vx{.5f}, vy{1};
        float speed = 200.f;
        
        void update(float delta_time);
        void bounce_x();
        void bounce_y();
    };
}
