#pragma once

#include <cstdint>
#include <vector>

#include "math.hpp"
#include "renderer/renderer.hpp"

namespace pong
{
    struct NumberStyle
    {
        float scale{1.0f};
        bool prefix_x{}; // draws an "x" glyph (the cell after "9") before the digits
    };

    class DamageNumbers
    {
    public:
        DamageNumbers() = default;
        DamageNumbers(Color normal, Color killed) : m_normal_color(normal), m_killed_color(killed) {}

        // Hits on the same box in quick succession are summed into one number.
        void add(const Rect& box, uint32_t amount, bool killed, NumberStyle style = {});
        void update(float delta_time);
        // digit_pixels is the atlas rect of the "0" glyph; 1-9 and "x" follow it to the right.
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
            NumberStyle style{};
        };

        std::vector<Number> m_numbers{};
        Color m_normal_color{255, 255, 255, 255};
        Color m_killed_color{255, 220, 80, 255};
    };
}
