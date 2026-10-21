#pragma once
#include <memory>
#include <SDL3/SDL_video.h>

#include "audio.hpp"
#include "game.hpp"
#include "save.hpp"
#include "renderer/webgpu/renderer.hpp"

namespace pong
{
    template <auto Fn>
    struct SdlDeleter
    {
        template <class T>
        void operator()(T* ptr) const noexcept { Fn(ptr); }
    };

    using WindowPtr = std::unique_ptr<SDL_Window, SdlDeleter<SDL_DestroyWindow>>;
    struct AppContext
    {
        WindowPtr window{};
        std::unique_ptr<WebGpuRenderer> renderer{};
        Game game{};
        Audio audio{};
        Save save{};
        float last_ms{};
        float accumulator{};
    };
}
