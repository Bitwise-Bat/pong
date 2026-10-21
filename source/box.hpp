#pragma once
#include "log.hpp"
#include "math.hpp"

namespace pong
{
    enum class BoxEffect : uint8_t { None, ExtraBall, Bigger };
    enum class Material : uint8_t { Stone, Iron, Bronze, Silver, Gold, Diamond, Count };

    // Money a caught drop of this material is worth, before the streak multiplier.
    constexpr uint32_t material_value(Material material)
    {
        switch (material)
        {
        case Material::Stone:   return 1;
        case Material::Iron:    return 3;
        case Material::Bronze:  return 6;
        case Material::Silver:  return 12;
        case Material::Gold:    return 25;
        case Material::Diamond: return 50;
        case Material::Count:   break;
        }
        return 0;
    }
    
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
