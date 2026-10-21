#pragma once
#include "log.hpp"
#include "math.hpp"

namespace pong
{
    enum class BoxEffect : uint8_t { None, ExtraBall, Bigger };
    
    struct Box
    {
        Rect rect{};
        Color base{50, 50, 50, 255};
        BoxEffect effect{};
        uint16_t health{2};
        uint16_t max_health{2};
        
        void hit(uint16_t damage)
        {
            health = std::max(0, health - damage);
        }
        
        [[nodiscard]] bool dead() const { return health <= 0;}
        
        [[nodiscard]] Color color() const
        {
            const float t = std::max(0.4f, static_cast<float>(health) / max_health);
            return { static_cast<std::uint8_t>(base.r * t),
                     static_cast<std::uint8_t>(base.g * t),
                     static_cast<std::uint8_t>(base.b * t),
                     base.a };
        }
    };
}
