#define SDL_MAIN_USE_CALLBACKS 1

#include "assert.hpp"

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <memory>

#include "application.hpp"
#include "game.hpp"
#include "log.hpp"
#include "web_bridge.hpp"
#include "renderer/webgpu/renderer.hpp"

namespace
{
    constexpr float FIXED_UPDATE_MS = 1000.0f / 50.0f;
    constexpr float FIXED_UPDATE_INTERVAL = FIXED_UPDATE_MS / 1000.0f;
    constexpr float MAX_FRAME_MS = 250.0f;

    using SurfacePtr = std::unique_ptr<SDL_Surface, pong::SdlDeleter<SDL_DestroySurface>>;

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

namespace
{
    void upload_atlas(pong::WebGpuRenderer& renderer)
    {
        SurfacePtr loaded{SDL_LoadPNG("/assets/textures/atlas.png")};
        P_SDL_ASSERT(loaded != nullptr);

        SurfacePtr rgba{SDL_ConvertSurface(loaded.get(), SDL_PIXELFORMAT_RGBA32)};
        P_SDL_ASSERT(rgba != nullptr);
        P_ASSERT(rgba->pitch == rgba->w * 4, "atlas rows are not tightly packed");

        const auto pixels = std::span(static_cast<const uint8_t*>(rgba->pixels), static_cast<size_t>(rgba->h) * rgba->pitch);
        renderer.upload_texture(0, 0, 0, rgba->w, rgba->h, pixels);
    }
}

SDL_AppResult SDL_AppInit(void** appstate, int argc, char* argv[])
{
    // SDL_SetHint(SDL_HINT_EMSCRIPTEN_KEYBOARD_ELEMENT, "#canvas");

    auto app = std::make_unique<pong::AppContext>();

    P_ASSERT(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO));

    app->window.reset(SDL_CreateWindow("Pong", 960, 540, 0));
    P_SDL_ASSERT(app->window.get() != nullptr);

    app->renderer = std::make_unique<pong::WebGpuRenderer>(960, 540);
    upload_atlas(*app->renderer);

    app->audio.init(app->game.events());
    app->save.init(app->game);

    app->game.restart();
    app->last_ms = static_cast<float>(SDL_GetTicksNS()) / 1.0e6f;

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
    auto app = static_cast<pong::AppContext*>(appstate);
    pong::WebGpuRenderer* renderer = app->renderer.get();

    const float now = static_cast<float>(SDL_GetTicksNS()) / 1.0e6f;
    float frame_ms = now - app->last_ms;
    app->last_ms = now;

    frame_ms = std::min(frame_ms, MAX_FRAME_MS);
    app->accumulator += frame_ms;

    while (app->accumulator >= FIXED_UPDATE_MS)
    {
        app->game.fixed_update(FIXED_UPDATE_INTERVAL);
        app->accumulator -= FIXED_UPDATE_MS;
    }

    app->game.update(frame_ms / 1000.0f);

    renderer->clear({10, 10, 150, 255});

    app->game.render(renderer);

    renderer->present();

    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void* appstate, SDL_AppResult result)
{
    delete static_cast<pong::AppContext*>(appstate);
}
