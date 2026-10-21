#pragma once

#include <array>
#include <cstdint>
#include <random>
#include <span>
#include <vector>

#include "ball.hpp"
#include "box.hpp"
#include "event_bus.hpp"
#include "field.hpp"
#include "input.hpp"
#include "paddle.hpp"
#include "particles.hpp"
#include "renderer/renderer.hpp"
#include "skills.hpp"

namespace pong
{
    class Game
    {
    public:
        EventBus& events() { return m_bus; }
        void update(float delta_time);
        void restart();
        void start();
        void render(Renderer* r);
        void fixed_update(float delta_time);

        [[nodiscard]] std::span<const Ball> balls() const { return m_balls; }

        [[nodiscard]] uint8_t skill_level(size_t id) const { return m_skill_levels[id]; }
        [[nodiscard]] bool can_buy_skill(size_t id) const;
        bool buy_skill(size_t id);

    private:
        std::mt19937 m_rng{ std::random_device{}() };
        std::discrete_distribution<uint8_t> m_buffPick{ 70, 25, 25 };
        std::discrete_distribution<uint8_t> m_materialPick{ 100, 15, 7, 4, 2, 1 };

        BoxEffect roll_buff() { return static_cast<BoxEffect>(m_buffPick(m_rng)); }
        Material roll_material() { return static_cast<Material>(m_materialPick(m_rng)); }

        void sample_paddle_input();
        void update_paddle(float delta_time);
        void update_balls(float delta_time);

        void collide_ball_walls(Ball& ball) const;
        void collide_ball_paddle(Ball& ball) const;
        void collide_balls_boxes();
        void collide_balls_balls();

        void apply_box_rewards(Ball& ball, const Box& box, std::vector<Ball>& spawned);
        void generate_boxes();
        void apply_skills(Ball& ball) const;

    private:
        Paddle m_paddle{};
        PaddleInput m_paddle_input{};
        std::vector<Ball> m_balls{};
        Input m_input{};
        Field m_field{};
        Particles m_particles{};
        bool m_playing{};
        std::array<uint32_t, static_cast<size_t>(Material::Count)> m_ores{};
        std::array<uint8_t, SKILLS.size()> m_skill_levels{};

        EventBus m_bus{};

        std::vector<Box> m_boxes{};
    };
}
