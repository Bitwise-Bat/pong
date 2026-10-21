#pragma once
#include "log.hpp"
#include "math.hpp"

namespace pong
{
    enum class BoxEffect : uint8_t { None, ExtraBall, Bigger };
    enum class Material : uint8_t { Stone, Iron, Bronze, Silver, Gold, Diamond, Count };
    
    struct Box
    {
        Rect rect{};
        BoxEffect effect{};
        Material material{};
        uint16_t health{2};
        uint16_t max_health{2};
        
        void hit(uint16_t damage)
        {
            health = std::max(0, health - damage);
        }
        
        [[nodiscard]] bool dead() const { return health <= 0;}
    };
}
