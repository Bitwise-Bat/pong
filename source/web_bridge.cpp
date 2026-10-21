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

    void on_skill_bought(const pong::SkillBought& event)
    {
        if (g_on_event.isUndefined()) return;

        emscripten::val obj = emscripten::val::object();
        obj.set("type", "skill");
        obj.set("id", event.id);

        g_on_event(obj);
    }

    emscripten::val bridge_get_skills()
    {
        emscripten::val list = emscripten::val::array();
        if (!g_ctx) return list;

        for (size_t i = 0; i < pong::SKILLS.size(); ++i)
        {
            const pong::SkillDef& def = pong::SKILLS[i];

            emscripten::val parents = emscripten::val::array();
            for (const int8_t parent : def.parents)
            {
                if (parent >= 0) parents.call<void>("push", static_cast<int>(parent));
            }

            emscripten::val obj = emscripten::val::object();
            obj.set("id", static_cast<uint32_t>(i));
            obj.set("name", std::string(def.name));
            obj.set("description", std::string(def.description));
            obj.set("x", def.x);
            obj.set("y", def.y);
            obj.set("parents", parents);
            obj.set("costMaterial", static_cast<uint8_t>(def.cost_material));
            obj.set("cost", def.cost);
            obj.set("owned", g_ctx->game.skill_owned(i));
            obj.set("canBuy", g_ctx->game.can_buy_skill(i));

            list.call<void>("push", obj);
        }

        return list;
    }

    bool bridge_buy_skill(uint32_t id)
    {
        return g_ctx && g_ctx->game.buy_skill(id);
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
    g_subs.push_back(g_ctx->game.events().subscribe<SkillBought>(&on_skill_bought));
}

EMSCRIPTEN_BINDINGS(pong)
{
    emscripten::function("onEvent", &bridge_on_event);
    emscripten::function("clearEvent", &bridge_clear_event);
    emscripten::function("startGame", &bridge_start_game);
    emscripten::function("getSkills", &bridge_get_skills);
    emscripten::function("buySkill", &bridge_buy_skill);
}
