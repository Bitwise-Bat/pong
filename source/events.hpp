#pragma once
#include <cstdint>

namespace pong
{
    struct OreChanged
    {
        uint8_t material{};
        uint32_t count{};
    };

    enum class GameOverReason : uint8_t { BallsLost, TimeUp };

    struct GameOver
    {
        GameOverReason reason{};
    };

    struct TimeChanged
    {
        uint32_t seconds{};
    };

    struct SkillBought
    {
        uint8_t id{};
    };

    struct BoxHit {};

    struct BoxDestroyed {};
}
