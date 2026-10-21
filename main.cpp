#define SDL_MAIN_USE_CALLBACKS 1

#include "assert.hpp"

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <memory>

#include "application.hpp"
#include "game.hpp"
#include "log.hpp"
#include "web_bridge.hpp"
#include "renderer/sdl/renderer.hpp"

namespace
{
    template <class... Args>
    [[noreturn]] void sdl_fatal(
        std::string_view cond, std::string_view file,
        int32_t line)
    {
        const char* sdl_err = SDL_GetError();
        ::pong::fatal_impl(std::format("{} {}:{} (SDL: {})",
                                       cond, file, line, sdl_err));
    }
}

#define P_SDL_ASSERT(cond, ...) \
    do { if(!(cond)) ::sdl_fatal(#cond, __FILE__, __LINE__ __VA_OPT__(,) __VA_ARGS__); } while(0)

SDL_AppResult SDL_AppInit(void** appstate, int argc, char* argv[])
{
    // SDL_SetHint(SDL_HINT_EMSCRIPTEN_KEYBOARD_ELEMENT, "#canvas");
    
    auto app = std::make_unique<AppContext>();

    P_ASSERT(SDL_Init(SDL_INIT_VIDEO));

    app->window.reset(SDL_CreateWindow("Pong", 800, 600, 0));
    P_SDL_ASSERT(app->window.get() != nullptr);
    
    app->renderer = std::make_unique<pong::SdlRenderer>(app->window.get());
    app->renderer->set_logical_size(800, 600);
    
    app->game.restart();
    app->last_ns = SDL_GetTicksNS();
    
    pong::bridge_attach(app.get());
    
    *appstate = app.release();

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void* appstate, SDL_Event* event)
{
    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void* appstate)
{
    auto app = static_cast<AppContext*>(appstate);
    pong::SdlRenderer* renderer = app->renderer.get();
    
    uint64_t now = SDL_GetTicksNS();
    float delta_time = (now - app->last_ns) / 1e9f;
    app->last_ns = now;
    
    app->game.update(delta_time);
    
    renderer->clear({20, 20, 20, 255});
    
    app->game.render(renderer);
    
    renderer->present();
    
    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void* appstate, SDL_AppResult result)
{
    delete static_cast<AppContext*>(appstate);
}
