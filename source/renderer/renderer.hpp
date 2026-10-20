#pragma once
#include "math.hpp"

namespace pong
{
    class Renderer
    {
    public:
        virtual ~Renderer() = default;
        virtual void clear(Color c) = 0;
        virtual void fill_rect(const Rect& rect, Color c) = 0;
    };
}
