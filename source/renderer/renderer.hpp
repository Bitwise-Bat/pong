#pragma once
#include "math.hpp"

namespace pong
{
    class Renderer
    {
    public:
        virtual ~Renderer() = default;
        virtual void clear(Color c) = 0;
        virtual void fill_rect(const Rect& rect, Color c, int32_t texture) = 0;
        void fill_rect(const Rect& rect, Color c) { fill_rect(rect, c, -1); }
    };
}
