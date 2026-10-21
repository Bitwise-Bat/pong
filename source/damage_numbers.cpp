#include "damage_numbers.hpp"

#include <algorithm>
#include <string>

#include "utils.hpp"

namespace pong
{
    namespace
    {
        constexpr float LIFE = 0.6f;
        constexpr float MERGE_WINDOW = 0.25f;
        constexpr float RISE_SPEED = 40.0f;
        constexpr float DIGIT_SIZE = 16.0f;
        constexpr float KILLED_DIGIT_SIZE = 22.0f;
        constexpr float DIGIT_ADVANCE = 0.7f; // share of the glyph size between digits
    }

    void DamageNumbers::add(const Rect& box, uint32_t amount, bool killed, NumberStyle style)
    {
        for (auto& n : m_numbers)
        {
            if (n.box_x != box.x || n.box_y != box.y || n.life < LIFE - MERGE_WINDOW) continue;

            n.amount += amount;
            n.killed = n.killed || killed;
            n.life = LIFE;
            return;
        }

        m_numbers.push_back({
            .box_x = box.x, .box_y = box.y,
            .x = box.x + box.w * 0.5f, .y = box.y + box.h * 0.5f,
            .amount = amount, .life = LIFE, .killed = killed, .style = style,
        });
    }

    void DamageNumbers::update(float delta_time)
    {
        for (size_t i = 0; i < m_numbers.size();)
        {
            auto& n = m_numbers[i];
            n.life -= delta_time;

            if (n.life <= 0.0f)
            {
                n = m_numbers.back();
                m_numbers.pop_back();
                continue;
            }

            n.y -= RISE_SPEED * delta_time;
            ++i;
        }
    }

    void DamageNumbers::render(Renderer* r, const Rect& digit_pixels) const
    {
        for (const auto& n : m_numbers)
        {
            const std::string text = (n.style.prefix_x ? "x" : "") + std::to_string(n.amount);
            const float size = (n.killed ? KILLED_DIGIT_SIZE : DIGIT_SIZE) * n.style.scale;
            const float advance = size * DIGIT_ADVANCE;
            const float width = advance * static_cast<float>(text.size() - 1) + size;

            // Fully visible for the first half of the life, then fades out.
            Color c = n.killed ? m_killed_color : m_normal_color;
            c.a = static_cast<uint8_t>(255.0f * std::min(1.0f, n.life / (LIFE * 0.5f)));

            float x = n.x - width * 0.5f;
            for (const char ch : text)
            {
                Rect glyph = digit_pixels;
                const int index = ch == 'x' ? 10 : ch - '0';
                glyph.x += static_cast<float>(index) * digit_pixels.w;

                r->fill_rect({x, n.y - size * 0.5f, size, size}, c, 0, pixels_to_uv(glyph));
                x += advance;
            }
        }
    }
}
