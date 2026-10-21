#pragma once
#include <span>

#include "math.hpp"

namespace pong
{
    class Renderer
    {
    public:
        static constexpr uint32_t TEXTURE_SIZE = 2048;
        static constexpr int32_t TEXTURE_LAYERS = 1;

        virtual ~Renderer() = default;
        virtual void clear(Color c) = 0;

        // uv is the normalized sub-rect of the texture layer (x, y, w, h in 0..1) mapped onto the rect.
        virtual void fill_rect(const Rect& rect, Color c, int32_t texture, const Rect& uv) = 0;
        void fill_rect(const Rect& rect, Color c) { fill_rect(rect, c, -1, {0.0f, 0.0f, 1.0f, 1.0f}); }

        // Copies tightly packed RGBA8 pixels into a TEXTURE_SIZE x TEXTURE_SIZE layer at (x, y).
        virtual void upload_texture(int32_t texture, uint32_t x, uint32_t y,
                                    uint32_t width, uint32_t height, std::span<const uint8_t> rgba) = 0;
    };
}
