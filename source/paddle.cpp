#include "paddle.hpp"

#include <algorithm>

namespace pong
{
    void Paddle::update(const PaddleInput& in, float delta_time, float field_width)
    {
        const float oldX = rect.x;
        rect.x += in.dir * speed * delta_time;
        rect.x = std::clamp(rect.x, 0.0f, field_width - rect.w);
        vx = (rect.x - oldX) / delta_time;    
    }
}
