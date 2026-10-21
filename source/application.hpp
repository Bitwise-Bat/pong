#pragma once
#include <memory>
#include <SDL3/SDL_video.h>

#include "game.hpp"
#include "renderer/webgpu/renderer.hpp"

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
    std::unique_ptr<pong::WebGpuRenderer> renderer{};
    pong::Game game{};
    float last_ms{};
    float accumulator{};
};
