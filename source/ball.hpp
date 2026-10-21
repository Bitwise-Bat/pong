#pragma once

#include "math.hpp"
#include "modifier.hpp"

#include <vector>

namespace pong
{
    class Ball
    {
    public:
        float x{}, y{};
        float vx{.0f}, vy{1};

        Stats base{};
        std::vector<Modifier> modifiers{};
        uint32_t combo{}; // boxes destroyed by this ball

        void update(float delta_time);
        void bounce_x();
        void bounce_y();

        void update_modifiers(float delta_time)
        {
            for (auto& m : modifiers) if (m.remaining > 0.0f) m.remaining -= delta_time;
            std::erase_if(modifiers, [](const Modifier& m) { return m.remaining != -1.0f && m.remaining <= 0.0f; });
        }
        
        [[nodiscard]] Rect hitbox() const
        {
            const auto bounds = stats()[Stat::Bounds];

            return {
                .x = x,
                .y = y,
                .w = bounds,
                .h = bounds
            };
        }

        // Drawn rect (Stat::Size), centered on the hitbox (Stat::Bounds).
        [[nodiscard]] Rect visual_rect() const
        {
            const Stats s = stats();
            const float size = s[Stat::Size];
            const float offset = (s[Stat::Bounds] - size) * 0.5f;

            return {
                .x = x + offset,
                .y = y + offset,
                .w = size,
                .h = size
            };
        }

        [[nodiscard]] Stats stats() const
        {
            Stats s = base;
            for (const auto& m : modifiers)
                if (m.op == Op::Add) s[m.stat] += m.value;
            for (const auto& m : modifiers)
                if (m.op == Op::Multiply) s[m.stat] *= m.value;
            return s;
        }
        
        [[nodiscard]] static Ball make_from(const Ball& ball)
        {
            Ball new_ball{};
        
            new_ball.x = ball.x;
            new_ball.y = ball.y;
        
            new_ball.base[Stat::Damage] = ball.base[Stat::Damage];
            new_ball.base[Stat::Size] = ball.base[Stat::Size];;
            new_ball.base[Stat::Bounds] = ball.base[Stat::Bounds];
            new_ball.base[Stat::Speed] = ball.base[Stat::Speed];;
            new_ball.base[Stat::Health] = ball.base[Stat::Health];;

            return new_ball;
        }
    };
}
