#define DR_MP3_IMPLEMENTATION
#include "audio.hpp"

#include <algorithm>

#include "assert.hpp"
#include "events.hpp"

namespace pong
{
    namespace
    {
        constexpr size_t MAX_VOICES = 32;
        constexpr float MUSIC_VOLUME = 0.4f;
        constexpr drmp3_uint64 MUSIC_DECODE_FRAMES = 4096;
    }

    Audio::~Audio()
    {
        if (m_stream) SDL_DestroyAudioStream(m_stream);
        if (m_music_stream) SDL_DestroyAudioStream(m_music_stream);
        if (m_music_open) drmp3_uninit(&m_music);
    }

    void Audio::init(EventBus& bus)
    {
        load(m_hit, "/assets/sounds/hit.wav");
        load(m_ore_destroy, "/assets/sounds/ore_destroy.wav");

        load_music("/assets/sounds/background.mp3");

        m_stream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &m_spec, &Audio::on_stream_needs_data, this);
        if (m_stream) SDL_ResumeAudioStreamDevice(m_stream);

        m_hit_sub = bus.subscribe<BoxHit>([this](const BoxHit&) { play(m_hit); });
        m_destroy_sub = bus.subscribe<BoxDestroyed>([this](const BoxDestroyed&) { play(m_ore_destroy); });
    }

    // All sounds are mixed into one stream, so they must share the format of the first one.
    void Audio::load(Sound& sound, const char* path)
    {
        SDL_AudioSpec spec{};
        Uint8* data = nullptr;
        P_ASSERT(SDL_LoadWAV(path, &spec, &data, &sound.size));
        sound.data.reset(data);

        if (m_spec.freq == 0) m_spec = spec;
        P_ASSERT(spec.format == m_spec.format && spec.channels == m_spec.channels && spec.freq == m_spec.freq);
    }

    void Audio::load_music(const char* path)
    {
        P_ASSERT(drmp3_init_file(&m_music, path, nullptr));
        m_music_open = true;

        const SDL_AudioSpec source{ SDL_AUDIO_S16, static_cast<int>(m_music.channels), static_cast<int>(m_music.sampleRate) };
        m_music_stream = SDL_CreateAudioStream(&source, &m_spec);
        P_ASSERT(m_music_stream != nullptr);
    }

    void Audio::on_stream_needs_data(void* userdata, SDL_AudioStream* stream, int additional_amount, int)
    {
        if (additional_amount > 0) static_cast<Audio*>(userdata)->mix(stream, additional_amount);
    }

    void Audio::play(const Sound& sound)
    {
        if (m_voices.size() < MAX_VOICES) m_voices.push_back({ &sound, 0 });
    }

    // Sums all playing sounds into one buffer so overlapping sounds start immediately instead of queueing.
    void Audio::mix(SDL_AudioStream* stream, int amount)
    {
        const int frame_size = (SDL_AUDIO_BITSIZE(m_spec.format) / 8) * m_spec.channels;
        amount -= amount % frame_size;

        std::vector<Uint8> buffer(static_cast<size_t>(amount), 0);
        mix_music(buffer);

        for (Voice& voice : m_voices)
        {
            const Uint32 chunk = std::min(static_cast<Uint32>(amount), voice.sound->size - voice.offset);
            SDL_MixAudio(buffer.data(), voice.sound->data.get() + voice.offset, m_spec.format, chunk, 1.0f);
            voice.offset += chunk;
        }
        std::erase_if(m_voices, [](const Voice& voice) { return voice.offset >= voice.sound->size; });

        SDL_PutAudioStreamData(stream, buffer.data(), amount);
    }

    // Decodes the mp3 on demand and loops it.
    void Audio::mix_music(std::vector<Uint8>& buffer)
    {
        const int size = static_cast<int>(buffer.size());
        std::vector<drmp3_int16> decoded(MUSIC_DECODE_FRAMES * m_music.channels);

        while (SDL_GetAudioStreamAvailable(m_music_stream) < size)
        {
            drmp3_uint64 frames = drmp3_read_pcm_frames_s16(&m_music, MUSIC_DECODE_FRAMES, decoded.data());
            if (frames == 0)
            {
                if (!drmp3_seek_to_pcm_frame(&m_music, 0)) return;
                continue;
            }
            SDL_PutAudioStreamData(m_music_stream, decoded.data(), static_cast<int>(frames * m_music.channels * sizeof(drmp3_int16)));
        }

        std::vector<Uint8> music(buffer.size());
        const int got = SDL_GetAudioStreamData(m_music_stream, music.data(), size);
        if (got > 0) SDL_MixAudio(buffer.data(), music.data(), m_spec.format, static_cast<Uint32>(got), MUSIC_VOLUME);
    }
}
