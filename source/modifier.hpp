#pragma once
#include <array>
#include <cstdint>

namespace pong
{
    // Damage..Bounds belong to the ball, Time is the round length, PaddleSpeed and Sprint belong to the paddle,
    // ComboCap is the highest ore multiplier a streak can reach.
    enum class Stat : uint8_t { Damage, Health, Speed, Size, Bounds, Time, PaddleSpeed, Sprint, ComboCap, Count };
    enum class Op : uint8_t { Add, Multiply };
    
    struct Modifier
    {
        Stat stat{};
        Op op{};
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
