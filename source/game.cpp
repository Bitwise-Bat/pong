#include "game.hpp"

#include <algorithm>
#include <cmath>

#include "collision.hpp"
#include "events.hpp"
#include "log.hpp"
#include "utils.hpp"

namespace pong
{
    namespace
    {
        constexpr Rect BACKGROUND_PIXELS{0.0f, 0.0f, 640.0f, 360.0f};
        constexpr float MATERIAL_TILE_SIZE = 50.0f;
        constexpr float MATERIALS_X = 650.0f;
        constexpr int MATERIAL_COUNT = 6;
        constexpr int DAMAGE_STAGES = 4;
        constexpr Color WHITE{255, 255, 255, 255};
        constexpr Color BLACK{0, 0, 0, 255};
        constexpr float BOX_CELL = 25.0f;
        constexpr float ROUND_TIME = 60.0f;
        constexpr float PADDLE_SPEED = 300.0f;
        constexpr uint16_t CHEST_HEALTH = 50;
        constexpr float DROP_SIZE = 25.0f;
        constexpr float DROP_GRAVITY = 500.0f;
        constexpr float DROP_MAX_FALL_SPEED = 260.0f;
        constexpr float DROP_SPREAD_SPEED = 60.0f;
        constexpr float DROP_POP_SPEED = 120.0f;
        constexpr uint32_t MAX_COMBO = 10;

        // Material weights (Stone..Diamond) for the row nearest the paddle and the deepest row; rows between blend them.
        constexpr std::array<float, static_cast<size_t>(Material::Count)> SHALLOW_MATERIAL_WEIGHTS{ 100, 3, 0, 0, 0, 0 };
        constexpr std::array<float, static_cast<size_t>(Material::Count)> DEEP_MATERIAL_WEIGHTS{ 25, 30, 22, 13, 7, 3 };

        // A drop is worth its material value times the ball's combo, up to the cap bought with the Streak skill.
        uint32_t combo_multiplier(const Ball& ball, uint32_t cap)
        {
            return std::clamp(ball.combo, 1u, cap);
        }

        uint16_t material_health(Material material)
        {
            switch (material)
            {
            case Material::Stone:   return 5;
            case Material::Iron:    return 10;
            case Material::Bronze:  return 20;
            case Material::Silver:  return 40;
            case Material::Gold:    return 60;
            case Material::Diamond: return 80;
            case Material::Count:   break;
            }
            return 1;
        }

        // Atlas row at y = 0, left to right: materials (enum order), damage overlays, paddle (3 cells), ball (1 cell), chest (1 cell), digits 0-9.
        Rect material_pixels(Material material)
        {
            const float x = MATERIALS_X + static_cast<float>(material) * MATERIAL_TILE_SIZE;
            return {x, 0.0f, MATERIAL_TILE_SIZE, MATERIAL_TILE_SIZE};
        }

        // Pickup icons sit in the row below their material tile.
        Rect drop_pixels(Material material)
        {
            const float x = MATERIALS_X + static_cast<float>(material) * MATERIAL_TILE_SIZE;
            return {x, MATERIAL_TILE_SIZE, MATERIAL_TILE_SIZE, MATERIAL_TILE_SIZE};
        }

        // Crack overlays follow the material tiles, ordered from lightly to heavily damaged.
        Rect damage_pixels(int stage)
        {
            const float x = MATERIALS_X + MATERIAL_COUNT * MATERIAL_TILE_SIZE + static_cast<float>(stage) * MATERIAL_TILE_SIZE;
            return {x, 0.0f, MATERIAL_TILE_SIZE, MATERIAL_TILE_SIZE};
        }

        constexpr float DAMAGE_END_X = MATERIALS_X + (MATERIAL_COUNT + DAMAGE_STAGES) * MATERIAL_TILE_SIZE;
        constexpr Rect PADDLE_PIXELS{DAMAGE_END_X, 0.0f, 3.0f * MATERIAL_TILE_SIZE, MATERIAL_TILE_SIZE};
        constexpr Rect BALL_PIXELS{DAMAGE_END_X + PADDLE_PIXELS.w, 0.0f, MATERIAL_TILE_SIZE, MATERIAL_TILE_SIZE};
        constexpr Rect CHEST_PIXELS{BALL_PIXELS.x + BALL_PIXELS.w, 0.0f, MATERIAL_TILE_SIZE, MATERIAL_TILE_SIZE};
        constexpr Rect DIGIT_PIXELS{CHEST_PIXELS.x + CHEST_PIXELS.w, 0.0f, MATERIAL_TILE_SIZE, MATERIAL_TILE_SIZE};

        // -1 for an undamaged box, otherwise 0..DAMAGE_STAGES-1 by the share of health lost.
        int damage_stage(const Box& box)
        {
            if (box.health >= box.max_health) return -1;

            const float lost = 1.0f - static_cast<float>(box.health) / static_cast<float>(box.max_health);
            return std::min(DAMAGE_STAGES - 1, static_cast<int>(lost * DAMAGE_STAGES));
        }

        Color material_color(Material material)
        {
            switch (material)
            {
            case Material::Stone:   return {140, 140, 140, 255};
            case Material::Iron:    return {190, 195, 205, 255};
            case Material::Bronze:  return {190, 120, 60, 255};
            case Material::Silver:  return {225, 230, 240, 255};
            case Material::Gold:    return {250, 205, 60, 255};
            case Material::Diamond: return {110, 230, 245, 255};
            case Material::Count:   break;
            }
            return WHITE;
        }

        void resolve_overlap(Ball& ball, float dx, float dy)
        {
            ball.x += dx;
            ball.y += dy;

            if (dx != 0.0f)      ball.vx = (dx < 0.0f) ? -std::abs(ball.vx) : std::abs(ball.vx);
            else if (dy != 0.0f) ball.vy = (dy < 0.0f) ? -std::abs(ball.vy) : std::abs(ball.vy);
        }
    }

    void Game::update(float delta_time)
    {
        update_paddle(delta_time);
        m_particles.update(delta_time);
        m_damage_numbers.update(delta_time);
        m_combo_numbers.update(delta_time);
    }

    void Game::restart()
    {
        m_paddle.rect.w = 100;
        m_paddle.rect.h = 12;
        m_paddle.width = 100;
        m_paddle.height = m_paddle.width * PADDLE_PIXELS.h / PADDLE_PIXELS.w;
        m_paddle.rect.x = m_field.width / 2 - m_paddle.rect.w / 2;

        m_paddle.stats = {};
        m_paddle.stats[Stat::PaddleSpeed] = PADDLE_SPEED;
        apply_skills(m_paddle.stats);

        Stats extra{};
        extra[Stat::ComboCap] = 1.0f;
        apply_skills(extra);
        m_combo_cap = std::clamp(static_cast<uint32_t>(extra[Stat::ComboCap]), 1u, MAX_COMBO);
        m_cursor_reach = extra[Stat::CursorReach];

        m_paddle.rect.y = m_field.height - 4 - (m_paddle.height + m_paddle.rect.h) / 2;

        m_balls.clear();
        m_boxes.clear();
        m_particles.clear();
        m_damage_numbers.clear();
        m_combo_numbers.clear();
        m_drops.clear();

        m_bus.publish(MoneyChanged{ .amount = m_money });

        Ball ball{};

        ball.x = m_field.width / 2 - 10 / 2;
        ball.y = m_field.height / 2 - 10 / 2;

        ball.base[Stat::Damage] = 2;
        ball.base[Stat::Size] = 20;
        ball.base[Stat::Bounds] = 10;
        ball.base[Stat::Speed] = 300;
        ball.base[Stat::Health] = 1;
        apply_skills(ball.base);

        m_balls.push_back(ball);

        generate_boxes();

        Stats round{};
        round[Stat::Time] = ROUND_TIME;
        apply_skills(round);
        m_time_left = round[Stat::Time];
        publish_time(true);
        publish_combo(true);
    }

    bool Game::can_buy_skill(size_t id) const
    {
        if (m_playing || id >= SKILLS.size()) return false;

        const SkillDef& skill = SKILLS[id];
        const size_t level = m_skill_levels[id];
        if (level >= skill.max_level()) return false;

        for (const int8_t parent : skill.parents)
        {
            if (parent >= 0 && m_skill_levels[static_cast<size_t>(parent)] == 0) return false;
        }

        return m_money >= skill.levels[level].cost;
    }

    bool Game::buy_skill(size_t id)
    {
        if (!can_buy_skill(id)) return false;

        m_money -= SKILLS[id].levels[m_skill_levels[id]].cost;
        ++m_skill_levels[id];

        m_bus.publish(MoneyChanged{ .amount = m_money });
        m_bus.publish(SkillBought{ .id = static_cast<uint8_t>(id) });
        return true;
    }

    // Same order as Ball::stats(): additions first, then multipliers.
    void Game::apply_skills(Stats& stats) const
    {
        for (const Op op : { Op::Add, Op::Multiply })
        {
            for (size_t i = 0; i < SKILLS.size(); ++i)
            {
                const SkillDef& skill = SKILLS[i];
                if (skill.op != op) continue;

                for (size_t level = 0; level < m_skill_levels[i]; ++level)
                {
                    const float value = skill.levels[level].value;
                    if (op == Op::Add) stats[skill.stat] += value;
                    else stats[skill.stat] *= value;
                }
            }
        }
    }

    void Game::render(Renderer* r)
    {
        // Fit the background to the field height, keeping its aspect; the right side may overflow.
        const float background_width = m_field.height * BACKGROUND_PIXELS.w / BACKGROUND_PIXELS.h;
        r->fill_rect({0.0f, 0.0f, background_width, m_field.height}, {255, 255, 255, 255},
                     0, pixels_to_uv(BACKGROUND_PIXELS));

        for (const auto& ball : m_balls)
        {
            r->fill_rect(ball.visual_rect(), WHITE, 0, pixels_to_uv(BALL_PIXELS));
        }

        // A box shows its material only when an empty cell touches it; the field walls and boxes hide it.
        const int cols = static_cast<int>(m_field.width / BOX_CELL);
        int rows = 0;
        for (const auto& box : m_boxes)
        {
            rows = std::max(rows, static_cast<int>(box.rect.y / BOX_CELL) + 1);
        }

        std::vector<bool> occupied(static_cast<size_t>(cols * rows), false);
        for (const auto& box : m_boxes)
        {
            const int x = static_cast<int>(box.rect.x / BOX_CELL);
            const int y = static_cast<int>(box.rect.y / BOX_CELL);
            occupied[static_cast<size_t>(y * cols + x)] = true;
        }

        const auto solid = [&](int x, int y)
        {
            if (x < 0 || x >= cols || y < 0) return true;
            if (y >= rows) return false;
            return static_cast<bool>(occupied[static_cast<size_t>(y * cols + x)]);
        };

        for (const auto& box : m_boxes)
        {
            const int x = static_cast<int>(box.rect.x / BOX_CELL);
            const int y = static_cast<int>(box.rect.y / BOX_CELL);
            if (solid(x - 1, y) && solid(x + 1, y) && solid(x, y - 1) && solid(x, y + 1))
            {
                r->fill_rect(box.rect, BLACK);
                continue;
            }

            r->fill_rect(box.rect, WHITE, 0, pixels_to_uv(material_pixels(box.material)));

            const int stage = damage_stage(box);
            if (stage >= 0)
            {
                r->fill_rect(box.rect, WHITE, 0, pixels_to_uv(damage_pixels(stage)));
            }

            if (box.effect != BoxEffect::None)
            {
                r->fill_rect(box.rect, WHITE, 0, pixels_to_uv(CHEST_PIXELS));
            }
        }

        for (const auto& drop : m_drops)
        {
            r->fill_rect(drop.rect, WHITE, 0, pixels_to_uv(drop_pixels(drop.material)));
        }

        r->fill_rect(m_paddle.visual_rect(), WHITE, 0, pixels_to_uv(PADDLE_PIXELS));

        m_particles.render(r);
        m_damage_numbers.render(r, DIGIT_PIXELS);
        m_combo_numbers.render(r, DIGIT_PIXELS);
    }

    void Game::reset_progress()
    {
        m_money = 0;
        m_skill_levels.fill(0);
        m_playing = false;
        restart();
    }

    void Game::start()
    {
        restart();
        m_playing = true;
    }

    void Game::fixed_update(float delta_time)
    {
        if (m_playing)
        {
            sample_paddle_input();
            update_balls(delta_time);
            update_drops(delta_time);

            m_time_left = std::max(0.0f, m_time_left - delta_time);
            publish_time(false);

            if (m_balls.empty())
            {
                end_round(GameOverReason::BallsLost);
            }
            else if (m_time_left <= 0.0f)
            {
                end_round(GameOverReason::TimeUp);
            }
            else
            {
                collide_balls_boxes();
                collide_balls_balls();
                publish_combo(false);
            }
        }

        m_bus.flush();
    }

    // Only whole-second changes are published, so the UI is not flooded every step.
    void Game::publish_time(bool force)
    {
        const auto seconds = static_cast<uint32_t>(std::ceil(m_time_left));
        if (!force && seconds == m_time_shown) return;

        m_time_shown = seconds;
        m_bus.publish(TimeChanged{ .seconds = seconds });
    }

    void Game::end_round(GameOverReason reason)
    {
        m_playing = false;
        m_drops.clear();
        m_bus.publish(GameOver{ .reason = reason });
    }

    void Game::sample_paddle_input()
    {
        m_paddle_input = {};
        if (m_input.down(Action::Left)) m_paddle_input.dir -= 1;
        if (m_input.down(Action::Right)) m_paddle_input.dir += 1;
        m_paddle_input.sprint = m_input.down(Action::Sprint);
    }

    void Game::update_paddle(float delta_time)
    {
        m_paddle.update(m_paddle_input, delta_time, m_field.width);
    }

    void Game::update_balls(float delta_time)
    {
        for (size_t i = 0; i < m_balls.size();)
        {
            auto& ball = m_balls[i];
            ball.update_modifiers(delta_time);
            ball.update(delta_time);

            if (ball.y > m_field.height)
            {
                m_balls[i] = m_balls.back();
                m_balls.pop_back();
                continue;
            }

            collide_ball_walls(ball);
            collide_ball_paddle(ball);
            ++i;
        }
    }

    void Game::collide_ball_walls(Ball& ball) const
    {
        const Rect r = ball.hitbox();

        if (r.x < 0.0f)
        {
            ball.x = 0.0f;
            ball.vx = std::abs(ball.vx);
        }
        else if (r.x + r.w > m_field.width)
        {
            ball.x = m_field.width - r.w;
            ball.vx = -std::abs(ball.vx);
        }

        if (r.y < 0.0f)
        {
            ball.y = 0.0f;
            ball.vy = std::abs(ball.vy);
        }
    }

    void Game::collide_ball_paddle(Ball& ball) const
    {
        float dx, dy;
        if (!overlap(ball.hitbox(), m_paddle.rect, dx, dy)) return;

        ball.x += dx;
        ball.y += dy;

        if (dx != 0.0f)
        {
            ball.vx = (dx < 0.0f) ? -std::abs(ball.vx) : std::abs(ball.vx);
        }
        else if (dy < 0.0f)
        {
            const Rect b = ball.hitbox();
            const Rect& p = m_paddle.rect;
            const float speed = std::sqrt(ball.vx * ball.vx + ball.vy * ball.vy);

            float hit = ((b.x + b.w * 0.5f) - (p.x + p.w * 0.5f)) / (p.w * 0.5f);
            hit = std::clamp(hit, -1.0f, 1.0f);

            float vx = hit * speed * 0.7f + m_paddle.vx * 0.5f;
            const float vy = -speed;

            const float min_vx = speed * 0.2f;
            const float max_vx = speed * 1.73f;
            if (std::abs(vx) < min_vx) vx = (vx < 0.0f) ? -min_vx : min_vx;
            vx = std::clamp(vx, -max_vx, max_vx);

            const float len = std::sqrt(vx * vx + vy * vy);
            ball.vx = vx / len * speed;
            ball.vy = vy / len * speed;
        }
    }

    void Game::collide_balls_boxes()
    {
        std::vector<Ball> spawned;
        std::vector<size_t> hits;
        std::vector<size_t> dead;

        for (auto& ball : m_balls)
        {
            const Stats stats = ball.stats();

            hits.clear();
            float best_dx = 0.0f, best_dy = 0.0f;
            for (size_t i = 0; i < m_boxes.size(); ++i)
            {
                float dx, dy;
                if (!overlap(ball.hitbox(), m_boxes[i].rect, dx, dy)) continue;

                hits.push_back(i);
                if (std::abs(dx) + std::abs(dy) > std::abs(best_dx) + std::abs(best_dy))
                {
                    best_dx = dx;
                    best_dy = dy;
                }
            }

            if (hits.empty()) continue;

            // Resolve once along the deepest overlap so touching several boxes does not reflect twice.
            resolve_overlap(ball, best_dx, best_dy);

            const Rect contact = ball.hitbox();
            const float normal = std::atan2(best_dy, best_dx);

            dead.clear();
            for (const size_t i : hits)
            {
                auto& box = m_boxes[i];
                const Color color = material_color(box.material);

                m_particles.emit(m_rng, {
                    .x = contact.x + contact.w * 0.5f, .y = contact.y + contact.h * 0.5f,
                    .angle = normal, .spread = 0.9f, .count = 4,
                    .color = color, .speed = 160.0f, .life = 0.35f, .size = 3.0f,
                });

                m_bus.publish(BoxHit{});

                const auto damage = static_cast<uint16_t>(stats[Stat::Damage]);
                box.hit(damage);
                m_damage_numbers.add(box.rect, damage, box.dead());
                if (!box.dead()) continue;

                m_bus.publish(BoxDestroyed{});

                m_particles.emit(m_rng, {
                    .x = box.rect.x + box.rect.w * 0.5f, .y = box.rect.y + box.rect.h * 0.5f,
                    .angle = 0.0f, .spread = 3.14159265f, .count = 16,
                    .color = color, .speed = 220.0f, .life = 0.7f, .size = 5.0f,
                });

                ++ball.combo;
                const uint32_t multiplier = combo_multiplier(ball, m_combo_cap);
                spawn_drop(box, multiplier);

                // The streak floats above the damage number as "x3", growing with the streak.
                if (multiplier > 1)
                {
                    const Rect above{box.rect.x, box.rect.y - box.rect.h * 0.7f, box.rect.w, box.rect.h};
                    const float scale = 1.0f + static_cast<float>(multiplier - 2) * 0.15f;
                    m_combo_numbers.add(above, multiplier, false, { .scale = scale, .prefix_x = true });
                }

                apply_box_rewards(ball, box, spawned);
                dead.push_back(i);
            }

            // Indices are ascending, so removing from the back keeps the remaining ones valid.
            for (auto it = dead.rbegin(); it != dead.rend(); ++it)
            {
                m_boxes[*it] = m_boxes.back();
                m_boxes.pop_back();
            }
        }

        m_balls.insert(m_balls.end(), spawned.begin(), spawned.end());
    }

    void Game::apply_box_rewards(Ball& ball, const Box& box, std::vector<Ball>& spawned)
    {
        ball.modifiers.push_back({ .stat = Stat::Speed, .op = Op::Add, .value = 10, .remaining = -1.f });

        switch (box.effect)
        {
        case BoxEffect::ExtraBall:
            spawned.push_back(Ball::make_from(ball));
            break;
        case BoxEffect::Bigger:
            ball.modifiers.push_back({ .stat = Stat::Size,   .op = Op::Add, .value = 2, .remaining = -1.f });
            ball.modifiers.push_back({ .stat = Stat::Bounds, .op = Op::Add, .value = 2, .remaining = -1.f });
            break;
        case BoxEffect::None:
            break;
        }
    }

    void Game::spawn_drop(const Box& box, uint32_t amount)
    {
        std::uniform_real_distribution<float> spread{-DROP_SPREAD_SPEED, DROP_SPREAD_SPEED};

        m_drops.push_back({
            .rect = {
                box.rect.x + (box.rect.w - DROP_SIZE) * 0.5f,
                box.rect.y + (box.rect.h - DROP_SIZE) * 0.5f,
                DROP_SIZE, DROP_SIZE,
            },
            .vx = spread(m_rng),
            .vy = -DROP_POP_SPEED,
            .material = box.material,
            .amount = amount,
        });
    }

    void Game::publish_combo(bool force)
    {
        uint32_t best = 1;
        for (const auto& ball : m_balls) best = std::max(best, combo_multiplier(ball, m_combo_cap));

        if (!force && best == m_combo_shown) return;

        m_combo_shown = best;
        m_bus.publish(ComboChanged{ .multiplier = best });
    }

    // True when the cursor is within the bought reach of the rect's nearest point.
    bool Game::grabbed_by_cursor(const Rect& rect) const
    {
        if (!m_pointer_valid || m_cursor_reach <= 0.0f) return false;

        const float x = std::clamp(m_pointer_x, rect.x, rect.x + rect.w) - m_pointer_x;
        const float y = std::clamp(m_pointer_y, rect.y, rect.y + rect.h) - m_pointer_y;
        return x * x + y * y <= m_cursor_reach * m_cursor_reach;
    }

    void Game::update_drops(float delta_time)
    {
        for (size_t i = 0; i < m_drops.size();)
        {
            auto& drop = m_drops[i];
            drop.vy = std::min(drop.vy + DROP_GRAVITY * delta_time, DROP_MAX_FALL_SPEED);
            drop.rect.x = std::clamp(drop.rect.x + drop.vx * delta_time, 0.0f, m_field.width - drop.rect.w);
            drop.rect.y += drop.vy * delta_time;

            float dx, dy;
            const bool caught = overlap(drop.rect, m_paddle.rect, dx, dy) || grabbed_by_cursor(drop.rect);
            if (caught)
            {
                m_money += material_value(drop.material) * drop.amount;
                m_bus.publish(MoneyChanged{ .amount = m_money });

                m_particles.emit(m_rng, {
                    .x = drop.rect.x + drop.rect.w * 0.5f, .y = drop.rect.y + drop.rect.h * 0.5f,
                    .angle = -1.5707963f, .spread = 1.0f, .count = 6,
                    .color = material_color(drop.material), .speed = 120.0f, .life = 0.4f, .size = 3.0f,
                });
            }

            if (caught || drop.rect.y > m_field.height)
            {
                drop = m_drops.back();
                m_drops.pop_back();
                continue;
            }

            ++i;
        }
    }

    void Game::collide_balls_balls()
    {
        for (size_t i = 0; i < m_balls.size(); ++i)
        {
            for (size_t j = i + 1; j < m_balls.size(); ++j)
            {
                auto& a = m_balls[i];
                auto& b = m_balls[j];

                float dx, dy;
                if (!overlap(a.hitbox(), b.hitbox(), dx, dy)) continue;

                resolve_overlap(a, dx * 0.5f, dy * 0.5f);
                resolve_overlap(b, -dx * 0.5f, -dy * 0.5f);
            }
        }
    }

    // depth is 0 for the row nearest the paddle and 1 for the deepest row.
    Material Game::roll_material(float depth)
    {
        std::array<float, static_cast<size_t>(Material::Count)> weights{};
        for (size_t i = 0; i < weights.size(); ++i)
        {
            weights[i] = std::lerp(SHALLOW_MATERIAL_WEIGHTS[i], DEEP_MATERIAL_WEIGHTS[i], depth);
        }

        std::discrete_distribution<uint8_t> pick(weights.begin(), weights.end());
        return static_cast<Material>(pick(m_rng));
    }

    void Game::generate_boxes()
    {
        constexpr size_t rows = 10;
        const size_t cols = static_cast<size_t>(m_field.width / BOX_CELL) + 1;

        for (size_t y = 0; y < rows; ++y)
        {
            for (size_t x = 0; x < cols; ++x)
            {
                Box box{};

                box.rect = { static_cast<float>(x) * BOX_CELL, static_cast<float>(y) * BOX_CELL, BOX_CELL, BOX_CELL };
                box.effect = roll_buff();

                // A box with an effect is a chest: a stone box with extra health.
                if (box.effect != BoxEffect::None)
                {
                    box.material = Material::Stone;
                    box.health = box.max_health = CHEST_HEALTH;
                }
                else
                {
                    const float depth = static_cast<float>(rows - 1 - y) / static_cast<float>(rows - 1);
                    box.material = roll_material(depth);
                    box.health = box.max_health = material_health(box.material);
                }

                m_boxes.push_back(box);
            }
        }
    }
}
