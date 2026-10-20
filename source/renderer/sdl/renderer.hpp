#pragma once
#include <memory>
#include <SDL3/SDL_render.h>

#include "assert.hpp"
#include "renderer/renderer.hpp"

namespace pong
{
    struct RenderCommand
    {
        
    };
    
    class SdlRenderer : public Renderer
    {
    private:
        struct Deleter { void operator()(SDL_Renderer* r) const { SDL_DestroyRenderer(r); }};
        std::unique_ptr<SDL_Renderer, Deleter> m_renderer{};
        
    public:
        explicit SdlRenderer(SDL_Window* window) : m_renderer(SDL_CreateRenderer(window, nullptr))
        {
            P_ASSERT(m_renderer, "SDL: {}", SDL_GetError());
        }
        
        void fill_rect(const Rect& rect, Color c) override;
        void clear(Color c) override;
        void present();
        
        void set_logical_size(uint32_t w, uint32_t h)
        {
            SDL_SetRenderLogicalPresentation(m_renderer.get(), w, h, SDL_LOGICAL_PRESENTATION_LETTERBOX);
        }
    };
}
