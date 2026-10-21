#include "particles.hpp"

#include <cmath>

namespace pong
{
    namespace
    {
        constexpr float GRAVITY = 400.0f;
    }

    void Particles::emit(std::mt19937& rng, const Burst& burst)
    {
        std::uniform_real_distribution<float> unit{0.0f, 1.0f};

        for (int i = 0; i < burst.count; ++i)
        {
            const float angle = burst.angle + (unit(rng) * 2.0f - 1.0f) * burst.spread;
            const float speed = burst.speed * (0.5f + unit(rng) * 0.5f);
            const float life = burst.life * (0.6f + unit(rng) * 0.4f);

            m_particles.push_back({
                .x = burst.x,
                .y = burst.y,
                .vx = std::cos(angle) * speed,
                .vy = std::sin(angle) * speed,
                .life = life,
                .max_life = life,
                .size = burst.size,
                .color = burst.color,
            });
        }
    }

    void Particles::update(float delta_time)
    {
        for (size_t i = 0; i < m_particles.size();)
        {
            auto& p = m_particles[i];
            p.life -= delta_time;

            if (p.life <= 0.0f)
            {
                p = m_particles.back();
                m_particles.pop_back();
                continue;
            }

            p.vy += GRAVITY * delta_time;
            p.x += p.vx * delta_time;
            p.y += p.vy * delta_time;
            ++i;
        }
    }

    void Particles::render(Renderer* r) const
    {
        for (const auto& p : m_particles)
        {
            const float t = p.life / p.max_life;
            const float size = p.size * (0.4f + 0.6f * t);

            Color c = p.color;
            c.a = static_cast<uint8_t>(static_cast<float>(c.a) * t);

            r->fill_rect({p.x - size * 0.5f, p.y - size * 0.5f, size, size}, c);
        }
    }
}
