#pragma once
#include <cstdint>

namespace pong
{
    struct BallChanged
    {
        uint32_t count{};
    };
    
    struct MoneyChanged
    {
        uint32_t money{};
    };
}
