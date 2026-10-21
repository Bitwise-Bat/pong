#pragma once

#include <SDL3/SDL_audio.h>
#include <SDL3/SDL_stdinc.h>

#include <memory>
#include <vector>

#include "event_bus.hpp"

namespace pong
{
    class Audio
    {
    public:
        Audio() = default;
        ~Audio();

        Audio(const Audio&) = delete;
        Audio& operator=(const Audio&) = delete;

        void init(EventBus& bus);

    private:
        struct SdlFree
        {
            void operator()(Uint8* ptr) const noexcept { SDL_free(ptr); }
        };

        static void SDLCALL on_stream_needs_data(void* userdata, SDL_AudioStream* stream, int additional_amount, int total_amount);

        struct Sound
        {
            std::unique_ptr<Uint8, SdlFree> data{};
            Uint32 size{};
        };

        struct Voice
        {
            const Sound* sound{};
            Uint32 offset{};
        };

        void load(Sound& sound, const char* path);
        void play(const Sound& sound);
        void mix(SDL_AudioStream* stream, int amount);

        SDL_AudioStream* m_stream{};
        SDL_AudioSpec m_spec{};
        Sound m_hit{};
        Sound m_ore_destroy{};
        std::vector<Voice> m_voices{};
        EventBus::Subscription m_hit_sub{};
        EventBus::Subscription m_destroy_sub{};
    };
}
