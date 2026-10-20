#pragma once

#include "ball.hpp"
#include "field.hpp"
#include "input.hpp"
#include "paddle.hpp"
#include "renderer/renderer.hpp"

#include <vector>
#include <span>

namespace pong
{
    class Game
    {
    public:
        void update(float delta_time);
        void restart();
        void render(Renderer* r);
        
        std::span<const Ball> balls() { return m_balls; }
    
    private:
        Paddle m_paddle{};
        std::vector<Ball> m_balls{};
        Input m_input{};
        Field m_field{};
    };
}
