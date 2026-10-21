#include "renderer/sdl/renderer.hpp"

namespace pong
{
    void SdlRenderer::clear(Color c)
    {
        SDL_SetRenderDrawColor(m_renderer.get(), c.r, c.g, c.b, c.a);
        SDL_RenderClear(m_renderer.get());
    }

    void SdlRenderer::present()
    {
        SDL_RenderPresent(m_renderer.get());
    }

    void SdlRenderer::fill_rect(const Rect& rect, Color c)
    {
        SDL_FRect sdl_rect{};
        sdl_rect.x = rect.x;
        sdl_rect.y = rect.y;
        sdl_rect.h = rect.h;
        sdl_rect.w = rect.w;
        
        SDL_SetRenderDrawColor(m_renderer.get(), c.r, c.g, c.b, c.a);
        SDL_RenderFillRect(m_renderer.get(), &sdl_rect);
    }
}
