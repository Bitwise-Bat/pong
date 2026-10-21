#pragma once

#include <random>
#include <vector>

#include "math.hpp"
#include "renderer/renderer.hpp"

namespace pong
{
    struct Burst
    {
        float x{}, y{};
        float angle{};   // radians, centre of the emission cone
        float spread{};  // half-width of the cone in radians; pi gives a full circle
        int count{};
        Color color{};
        float speed{};   // px/s, each particle gets 50-100% of it
        float life{};    // seconds, each particle gets 60-100% of it
        float size{};    // px, starting edge length
    };

    class Particles
    {
    public:
        void emit(std::mt19937& rng, const Burst& burst);
        void update(float delta_time);
        void render(Renderer* r) const;
        void clear() { m_particles.clear(); }

    private:
        struct Particle
        {
            float x{}, y{};
            float vx{}, vy{};
            float life{}, max_life{};
            float size{};
            Color color{};
        };

        std::vector<Particle> m_particles{};
    };
}
