#pragma once

#include "math.hpp"
#include "modifier.hpp"

namespace pong
{
    struct PaddleInput
    {
        float dir{0.0f};
        bool sprint{false};
    };
    
    class Paddle
    {
    public:
        Rect rect{}; // collision bounds
        float width{}, height{}; // drawn size
        Stats stats{}; // PaddleSpeed, and Sprint as the extra speed while sprinting (0 until unlocked)
        float vx{0.0};
        
        void update(const PaddleInput& in, float delta_time, float field_width);

        // Drawn rect, centered on the bounds.
        [[nodiscard]] Rect visual_rect() const
        {
            return {
                .x = rect.x + (rect.w - width) * 0.5f,
                .y = rect.y + (rect.h - height) * 0.5f,
                .w = width,
                .h = height
            };
        }
    };
}
