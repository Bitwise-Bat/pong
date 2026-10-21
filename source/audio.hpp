#pragma once

#include <SDL3/SDL_audio.h>
#include <SDL3/SDL_stdinc.h>
#include <dr_mp3.h>

#include <array>
#include <atomic>
#include <memory>
#include <vector>

#include "event_bus.hpp"

namespace pong
{
    enum class Volume : uint8_t { Main, Music, Sfx, Count };

    class Audio
    {
    public:
        Audio() = default;
        ~Audio();

        Audio(const Audio&) = delete;
        Audio& operator=(const Audio&) = delete;

        void init(EventBus& bus);

        [[nodiscard]] float volume(Volume channel) const { return m_volumes[static_cast<size_t>(channel)]; }
        // value is clamped to 0..1.
        void set_volume(Volume channel, float value);

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
        void load_music(const char* path);
        void play(const Sound& sound);
        void mix(SDL_AudioStream* stream, int amount);
        void mix_music(std::vector<Uint8>& buffer);

        SDL_AudioStream* m_stream{};
        SDL_AudioSpec m_spec{};
        Sound m_hit{};
        Sound m_ore_destroy{};
        std::vector<Voice> m_voices{};
        drmp3 m_music{};
        bool m_music_open{};
        // Converts the decoded mp3 to the format of the output stream.
        SDL_AudioStream* m_music_stream{};
        EventBus::Subscription m_hit_sub{};
        EventBus::Subscription m_destroy_sub{};
        // Read by the audio callback.
        std::array<std::atomic<float>, static_cast<size_t>(Volume::Count)> m_volumes{ 1.0f, 1.0f, 1.0f };
    };
}
