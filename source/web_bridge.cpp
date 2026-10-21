#include "web_bridge.hpp"
#include "application.hpp"
#include "event_bus.hpp"

#include <emscripten/bind.h>

#include "events.hpp"
#include "log.hpp"

namespace
{
    pong::AppContext* g_ctx{};
    emscripten::val g_on_event = emscripten::val::undefined();
    
    std::vector<pong::EventBus::Subscription> g_subs{};

    void on_ore_changed(const pong::OreChanged& event)
    {
        if (g_on_event.isUndefined()) return;

        emscripten::val obj = emscripten::val::object();
        obj.set("type", "ore");
        obj.set("material", event.material);
        obj.set("count", event.count);

        g_on_event(obj);
    }

    void on_game_over(const pong::GameOver&)
    {
        if (g_on_event.isUndefined()) return;

        emscripten::val obj = emscripten::val::object();
        obj.set("type", "gameOver");

        g_on_event(obj);
    }

    void bridge_start_game()
    {
        if (g_ctx) g_ctx->game.start();
    }

    void bridge_on_event(emscripten::val cb)
    {
        g_on_event = cb;
    }

    void bridge_clear_event()
    {
        g_on_event = emscripten::val::undefined();
    }
}

void pong::bridge_attach(AppContext* ctx)
{
    g_ctx = ctx;
    
    g_subs.push_back(g_ctx->game.events().subscribe<OreChanged>(&on_ore_changed));
    g_subs.push_back(g_ctx->game.events().subscribe<GameOver>(&on_game_over));
}

EMSCRIPTEN_BINDINGS(pong)
{
    emscripten::function("onEvent", &bridge_on_event);
    emscripten::function("clearEvent", &bridge_clear_event);
    emscripten::function("startGame", &bridge_start_game);
}
