#pragma once
#include <cstdint>

namespace pong
{
    struct BallChanged
    {
        uint32_t count{};
    };
    
    struct ScoreChanged
    {
        uint32_t score{};
    };
}
