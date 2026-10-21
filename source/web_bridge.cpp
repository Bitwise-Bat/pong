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

    void on_ball_changed(const pong::BallChanged& event)
    {
        if (g_on_event.isUndefined()) return; 
    
        emscripten::val obj = emscripten::val::object();
        obj.set("type", "balls");
        obj.set("count", event.count);
    
        g_on_event(obj);
    }

    void on_score_changed(const pong::MoneyChanged& event)
    {
        if (g_on_event.isUndefined()) return;
    
        emscripten::val obj = emscripten::val::object();
        obj.set("type", "money");
        obj.set("money", event.money);
    
        g_on_event(obj);
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
    
    g_subs.push_back(g_ctx->game.events().subscribe<BallChanged>(&on_ball_changed));
    g_subs.push_back(g_ctx->game.events().subscribe<MoneyChanged>(&on_score_changed));
}

EMSCRIPTEN_BINDINGS(pong)
{
    emscripten::function("onEvent", &bridge_on_event);
    emscripten::function("clearEvent", &bridge_clear_event);
}
