#include "game.hpp"

#include <complex>

#include "collision.hpp"
#include "log.hpp"

namespace pong
{
    void Game::update(float delta_time)
    {
        PaddleInput paddle_input{};
        if (m_input.down(Action::Left)) paddle_input.dir -= 1;
        if (m_input.down(Action::Right)) paddle_input.dir += 1;
        
        m_paddle.update(paddle_input, delta_time, m_field.width);
        
        for (size_t i = 0; i < m_balls.size();)
        {
            auto& ball = m_balls[i];
            
            ball.update(delta_time);
            Rect& r = ball.rect;
            
            if (r.y > 600)
            {
                m_balls[i] = m_balls.back();
                m_balls.pop_back();
                continue;
            }
            
            ++i;

            if (r.x < 0.0f)
            {
                r.x = 0.0f;
                ball.vx = std::abs(ball.vx);
            }
            else if (r.x + r.w > m_field.width)
            {
                r.x = m_field.width - r.w;
                ball.vx = -std::abs(ball.vx);
            }

            if (r.y < 0.0f)
            {
                ball.vy = -ball.vy;
            }

            if (r.y > m_field.height)
            {
                r.y = m_field.height - r.h;
                ball.vy = -std::abs(ball.vy);
            }
        
            float dx, dy;
            if (overlap(ball.rect, m_paddle.rect, dx, dy))
            {
                ball.rect.x += dx;
                ball.rect.y += dy;
            
                if (dx != 0.0f)
                {
                    ball.vx = (dx < 0.0f) ? -std::abs(ball.vx) : std::abs(ball.vx);
                }
                else if (dy < 0.0f)
                {
                    const Rect& b = ball.rect;
                    const Rect& p = m_paddle.rect;
                    const float speed = std::sqrt(ball.vx * ball.vx + ball.vy * ball.vy);

                    float hit = ((b.x + b.w * 0.5f) - (p.x + p.w * 0.5f)) / (p.w * 0.5f);
                    hit = std::clamp(hit, -1.0f, 1.0f);

                    float vx = hit * speed * 0.7f + m_paddle.vx * 0.5f;
                    float vy = -speed;

                    const float minVx = speed * 0.2f;
                    const float maxVx = speed * 1.73f;
                    if (std::abs(vx) < minVx) vx = (vx < 0.0f) ? -minVx : minVx;
                    vx = std::clamp(vx, -maxVx, maxVx);

                    const float len = std::sqrt(vx * vx + vy * vy);
                    ball.vx = vx / len * speed;
                    ball.vy = vy / len * speed;
                }
            }
        }
    }

    void Game::restart()
    {
        m_paddle.rect.w = 100;
        m_paddle.rect.h = 12;
        m_paddle.rect.x = 400 - m_paddle.rect.w / 2;
        m_paddle.rect.y = 600 - m_paddle.rect.h / 2 - 10;
        
        m_balls.clear();
        Ball ball{};
        
        ball.rect.w = 10;
        ball.rect.h = 10;
        ball.rect.x = 400 - ball.rect.w / 2;
        ball.rect.y = 300 - ball.rect.h / 2;
        
        m_balls.push_back(ball);
    };

    void Game::render(Renderer* r)
    {
        for (const auto& ball : m_balls)
        {   
            r->fill_rect(ball.rect, {255, 255, 255, 255});
        }
        
        r->fill_rect(m_paddle.rect, {255, 255, 255, 255});
    };
}
