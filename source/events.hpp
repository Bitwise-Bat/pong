#pragma once
#include <cstdint>

namespace pong
{
    struct OreChanged
    {
        uint8_t material{};
        uint32_t count{};
    };

    struct GameOver
    {
        uint32_t score{};
    };

    struct MoneyChanged
    {
        uint32_t money{};
    };
}
