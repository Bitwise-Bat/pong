#pragma once

#include <cstdint>
#include <vector>

#include "math.hpp"
#include "renderer/renderer.hpp"

namespace pong
{
    class DamageNumbers
    {
    public:
        // Hits on the same box in quick succession are summed into one number.
        void add(const Rect& box, uint32_t amount, bool killed);
        void update(float delta_time);
        // digit_pixels is the atlas rect of the "0" glyph; 1-9 follow it to the right.
        void render(Renderer* r, const Rect& digit_pixels) const;
        void clear() { m_numbers.clear(); }

    private:
        struct Number
        {
            float box_x{}, box_y{};
            float x{}, y{};
            uint32_t amount{};
            float life{};
            bool killed{};
        };

        std::vector<Number> m_numbers{};
    };
}
