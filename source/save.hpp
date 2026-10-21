#pragma once

#include "event_bus.hpp"

namespace pong
{
    class Game;

    // Keeps ores and skill levels in the browser's localStorage.
    class Save
    {
    public:
        // Loads the stored progress into the game, then saves after every skill purchase, round end and page close.
        void init(Game& game);
        void save() const;

    private:
        void load();

        Game* m_game{};
        EventBus::Subscription m_skill_sub{};
        EventBus::Subscription m_round_sub{};
    };
}
