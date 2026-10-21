#pragma once

#include <algorithm>
#include <array>
#include <cstdint>
#include <random>
#include <span>
#include <vector>

#include "ball.hpp"
#include "box.hpp"
#include "damage_numbers.hpp"
#include "event_bus.hpp"
#include "events.hpp"
#include "field.hpp"
#include "input.hpp"
#include "paddle.hpp"
#include "particles.hpp"
#include "renderer/renderer.hpp"
#include "skills.hpp"

namespace pong
{
    // Ore falling from a destroyed box; it counts only when the paddle catches it.
    struct Drop
    {
        Rect rect{};
        float vx{}, vy{};
        Material material{};
        uint32_t amount{1};
    };

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
        [[nodiscard]] uint32_t ore(size_t material) const { return m_ores[material]; }

        // Used when restoring a save; levels above the skill's maximum are clamped.
        void set_ore(size_t material, uint32_t count) { m_ores[material] = count; }
        void set_skill_level(size_t id, uint32_t level)
        {
            m_skill_levels[id] = static_cast<uint8_t>(std::min<size_t>(level, SKILLS[id].max_level()));
        }
        [[nodiscard]] bool can_buy_skill(size_t id) const;
        bool buy_skill(size_t id);

        // Cursor position as a 0..1 share of the field, used to grab drops once Grabby Hands is bought.
        void set_pointer(float u, float v) { m_pointer_x = u * m_field.width; m_pointer_y = v * m_field.height; m_pointer_valid = true; }
        void clear_pointer() { m_pointer_valid = false; }

    private:
        std::mt19937 m_rng{ std::random_device{}() };
        std::discrete_distribution<uint8_t> m_buffPick{ 98, 1, 1 };
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
        void spawn_drop(const Box& box, uint32_t amount);
        void publish_combo(bool force);
        void update_drops(float delta_time);
        [[nodiscard]] bool grabbed_by_cursor(const Rect& rect) const;
        void generate_boxes();
        void apply_skills(Stats& stats) const;
        void publish_time(bool force);
        void end_round(GameOverReason reason);

    private:
        Paddle m_paddle{};
        PaddleInput m_paddle_input{};
        std::vector<Ball> m_balls{};
        Input m_input{};
        Field m_field{};
        Particles m_particles{};
        DamageNumbers m_damage_numbers{};
        DamageNumbers m_combo_numbers{ {110, 230, 245, 255}, {110, 230, 245, 255} };
        bool m_playing{};
        float m_time_left{};
        uint32_t m_combo_shown{};
        uint32_t m_combo_cap{1};
        float m_cursor_reach{};
        float m_pointer_x{}, m_pointer_y{};
        bool m_pointer_valid{};
        uint32_t m_time_shown{};
        std::array<uint32_t, static_cast<size_t>(Material::Count)> m_ores{};
        std::array<uint8_t, SKILLS.size()> m_skill_levels{};

        EventBus m_bus{};

        std::vector<Box> m_boxes{};
        std::vector<Drop> m_drops{};
    };
}
