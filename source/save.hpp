#pragma once

#include "event_bus.hpp"

namespace pong
{
    class Audio;
    class Game;

    // Keeps money, skill levels and volume settings in the browser's localStorage.
    class Save
    {
    public:
        // Loads the stored data, then saves after every skill purchase, round end and page close.
        void init(Game& game, Audio& audio);
        void save() const;
        // Deletes the stored data and resets progress and volumes to their defaults.
        void reset();

    private:
        void load();

        Game* m_game{};
        Audio* m_audio{};
        EventBus::Subscription m_skill_sub{};
        EventBus::Subscription m_round_sub{};
    };
}
