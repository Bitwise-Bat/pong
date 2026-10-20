#pragma once
#include "math.hpp"

#include <algorithm>

namespace pong
{
    inline bool overlap(const Rect& a, const Rect& b, float& dx, float& dy)
    {
        const float ox = std::min(a.x + a.w, b.x + b.w) - std::max(a.x, b.x);
        const float oy = std::min(a.y + a.h, b.y + b.h) - std::max(a.y, b.y);
        if (ox <= 0.0f || oy <= 0.0f) return false;

        const float acx = a.x + a.w * 0.5f, bcx = b.x + b.w * 0.5f;
        const float acy = a.y + a.h * 0.5f, bcy = b.y + b.h * 0.5f;

        if (ox < oy) { dx = (acx < bcx) ? -ox : ox; dy = 0.0f; } 
        else         { dy = (acy < bcy) ? -oy : oy; dx = 0.0f; }
        return true;
    }
}
