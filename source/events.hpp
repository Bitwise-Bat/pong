#pragma once
#include <cstdint>

namespace pong
{
    struct MoneyChanged
    {
        uint32_t amount{};
    };

    enum class GameOverReason : uint8_t { BallsLost, TimeUp };

    struct GameOver
    {
        GameOverReason reason{};
    };

    // The highest money multiplier among the balls in play.
    struct ComboChanged
    {
        uint32_t multiplier{};
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
