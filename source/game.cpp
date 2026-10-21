#include "game.hpp"

#include <algorithm>
#include <cmath>

#include "collision.hpp"
#include "events.hpp"
#include "log.hpp"

namespace pong
{
    namespace
    {
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
        update_balls(delta_time);
        collide_balls_boxes();
        collide_balls_balls();

        m_bus.flush();
    }

    void Game::restart()
    {
        m_paddle.rect.w = 100;
        m_paddle.rect.h = 12;
        m_paddle.rect.x = m_field.width / 2 - m_paddle.rect.w / 2;
        m_paddle.rect.y = m_field.height - m_paddle.rect.h / 2 - 10;

        m_balls.clear();
        Ball ball{};

        ball.x = m_field.width / 2 - 10 / 2;
        ball.y = m_field.height / 2 - 10 / 2;

        ball.base[Stat::Damage] = 2;
        ball.base[Stat::Size] = 10;
        ball.base[Stat::Speed] = 300;
        ball.base[Stat::Health] = 1;
        ball.base[Stat::Money] = 1;

        m_balls.push_back(ball);

        generate_boxes();
    }

    void Game::render(Renderer* r)
    {
        for (const auto& ball : m_balls)
        {
            r->fill_rect(ball.hitbox(), {255, 255, 255, 255});
        }

        for (const auto& box : m_boxes)
        {
            r->fill_rect(box.rect, box.color());
        }

        r->fill_rect(m_paddle.rect, {255, 255, 255, 255});
    }

    void Game::update_paddle(float delta_time)
    {
        PaddleInput paddle_input{};
        if (m_input.down(Action::Left)) paddle_input.dir -= 1;
        if (m_input.down(Action::Right)) paddle_input.dir += 1;

        m_paddle.update(paddle_input, delta_time, m_field.width);
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

        for (auto& ball : m_balls)
        {
            const Stats stats = ball.stats();

            for (size_t i = 0; i < m_boxes.size(); ++i)
            {
                auto& box = m_boxes[i];

                float dx, dy;
                if (!overlap(ball.hitbox(), box.rect, dx, dy)) continue;

                resolve_overlap(ball, dx, dy);

                box.hit(static_cast<uint16_t>(stats[Stat::Damage]));
                if (box.dead())
                {
                    m_score += static_cast<uint8_t>(stats[Stat::Money]);
                    m_bus.publish(MoneyChanged{ .money = m_score });

                    apply_box_rewards(ball, box, spawned);

                    m_boxes[i] = m_boxes.back();
                    m_boxes.pop_back();
                }
                break;
            }
        }

        m_balls.insert(m_balls.end(), spawned.begin(), spawned.end());
    }

    void Game::apply_box_rewards(Ball& ball, const Box& box, std::vector<Ball>& spawned)
    {
        ball.modifiers.push_back({ .stat = Stat::Money, .op = Op::Add, .value = 1,  .remaining = -1.f });
        ball.modifiers.push_back({ .stat = Stat::Speed, .op = Op::Add, .value = 10, .remaining = -1.f });

        switch (box.effect)
        {
        case BoxEffect::ExtraBall:
            spawned.push_back(Ball::make_from(ball));
            break;
        case BoxEffect::Bigger:
            ball.modifiers.push_back({ .stat = Stat::Size,   .op = Op::Add, .value = 10, .remaining = -1.f });
            ball.modifiers.push_back({ .stat = Stat::Damage, .op = Op::Add, .value = 1,  .remaining = 100.f });
            break;
        case BoxEffect::None:
            break;
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

    void Game::generate_boxes()
    {
        constexpr size_t rows = 10;
        constexpr float cell = 20.0f;
        const size_t cols = static_cast<size_t>(m_field.width / cell);

        for (size_t y = 0; y < rows; ++y)
        {
            for (size_t x = 0; x < cols; ++x)
            {
                Box box{};

                box.rect = { static_cast<float>(x) * cell, static_cast<float>(y) * cell, cell, cell };
                box.effect = roll_buff();

                m_boxes.push_back(box);
            }
        }
    }
}
