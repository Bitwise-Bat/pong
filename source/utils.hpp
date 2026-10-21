#pragma once
#include "math.hpp"
#include "renderer/renderer.hpp"

namespace pong
{
    // Converts a pixel region of a texture layer (x, y, w, h) into the normalized uv rect fill_rect expects.
    [[nodiscard]] constexpr Rect pixels_to_uv(const Rect& pixels, float texture_size = Renderer::TEXTURE_SIZE)
    {
        return { pixels.x / texture_size, pixels.y / texture_size,
                 pixels.w / texture_size, pixels.h / texture_size };
    }
}
