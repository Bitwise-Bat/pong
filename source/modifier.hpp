#pragma once
#include <array>
#include <cstdint>

namespace pong
{
    enum class Stat : uint8_t { Damage, Health, Speed, Size, Bounds, Count };
    enum class Op : uint8_t { Add, Multiply };
    
    struct Modifier
    {
        Stat stat;
        Op op;
        float value{};
        float remaining = -1.f; // -1 infinity
    };
    
    struct Stats
    {
        std::array<float, static_cast<size_t>(Stat::Count)> v{};
        float operator[](Stat s) const { return v[static_cast<size_t>(s)]; }
        float& operator[](Stat s) { return v[static_cast<size_t>(s)]; }
    };
}
