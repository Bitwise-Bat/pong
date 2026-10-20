#pragma once

#include "math.hpp"

namespace pong
{
    struct PaddleInput
    {
        float dir{0.0f};
    };
    
    class Paddle
    {
    public:
        Rect rect{};
        float speed = 400.f;
        float vx{0.0};
        
        void update(const PaddleInput& in, float delta_time, float field_width);
    };
}
