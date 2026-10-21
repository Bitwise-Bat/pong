#pragma once

#include <cstdint>

namespace pong
{
    enum class Action : uint8_t { Left, Right, Sprint };
    
    class Input
    {
    public:
        bool down(Action action) const;
    };
}
