#pragma once
#include <cstdint>

namespace pong
{
    struct OreChanged
    {
        uint8_t material{};
        uint32_t count{};
    };

    struct GameOver {};

    struct BoxHit {};

    struct BoxDestroyed {};
}
