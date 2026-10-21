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

    void on_game_over(const pong::GameOver& event)
    {
        if (g_on_event.isUndefined()) return;

        emscripten::val obj = emscripten::val::object();
        obj.set("type", "gameOver");
        obj.set("reason", event.reason == pong::GameOverReason::TimeUp ? "time" : "balls");

        g_on_event(obj);
    }

    void on_combo_changed(const pong::ComboChanged& event)
    {
        if (g_on_event.isUndefined()) return;

        emscripten::val obj = emscripten::val::object();
        obj.set("type", "combo");
        obj.set("multiplier", event.multiplier);

        g_on_event(obj);
    }

    void on_time_changed(const pong::TimeChanged& event)
    {
        if (g_on_event.isUndefined()) return;

        emscripten::val obj = emscripten::val::object();
        obj.set("type", "time");
        obj.set("seconds", event.seconds);

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
            const size_t level = g_ctx->game.skill_level(i);
            const size_t max_level = def.max_level();

            emscripten::val values = emscripten::val::array();
            for (size_t l = 0; l < max_level; ++l)
            {
                values.call<void>("push", def.levels[l].value);
            }

            obj.set("statName", std::string(def.stat_name));
            obj.set("multiply", def.op == pong::Op::Multiply);
            obj.set("x", def.x);
            obj.set("y", def.y);
            obj.set("parents", parents);
            obj.set("costMaterial", static_cast<uint8_t>(def.cost_material));
            obj.set("level", static_cast<uint32_t>(level));
            obj.set("maxLevel", static_cast<uint32_t>(max_level));
            obj.set("values", values);
            obj.set("cost", level < max_level ? def.levels[level].cost : 0u);
            obj.set("canBuy", g_ctx->game.can_buy_skill(i));

            list.call<void>("push", obj);
        }

        return list;
    }

    bool bridge_buy_skill(uint32_t id)
    {
        return g_ctx && g_ctx->game.buy_skill(id);
    }

    emscripten::val bridge_get_volumes()
    {
        emscripten::val obj = emscripten::val::object();
        if (!g_ctx) return obj;

        obj.set("main", g_ctx->audio.volume(pong::Volume::Main));
        obj.set("music", g_ctx->audio.volume(pong::Volume::Music));
        obj.set("sfx", g_ctx->audio.volume(pong::Volume::Sfx));
        return obj;
    }

    // channel follows pong::Volume: 0 main, 1 music, 2 sfx.
    void bridge_set_volume(uint32_t channel, float value)
    {
        if (!g_ctx || channel >= static_cast<uint32_t>(pong::Volume::Count)) return;

        g_ctx->audio.set_volume(static_cast<pong::Volume>(channel), value);
        g_ctx->save.save();
    }

    void bridge_start_game()
    {
        if (g_ctx) g_ctx->game.start();
    }

    void bridge_on_event(emscripten::val cb)
    {
        g_on_event = cb;
        if (!g_ctx) return;

        // The ores restored from the save were published before JS was listening.
        for (size_t i = 0; i < static_cast<size_t>(pong::Material::Count); ++i)
        {
            on_ore_changed({ .material = static_cast<uint8_t>(i), .count = g_ctx->game.ore(i) });
        }
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
    g_subs.push_back(g_ctx->game.events().subscribe<TimeChanged>(&on_time_changed));
    g_subs.push_back(g_ctx->game.events().subscribe<ComboChanged>(&on_combo_changed));
    g_subs.push_back(g_ctx->game.events().subscribe<SkillBought>(&on_skill_bought));
}

EMSCRIPTEN_BINDINGS(pong)
{
    emscripten::function("onEvent", &bridge_on_event);
    emscripten::function("clearEvent", &bridge_clear_event);
    emscripten::function("startGame", &bridge_start_game);
    emscripten::function("getSkills", &bridge_get_skills);
    emscripten::function("buySkill", &bridge_buy_skill);
    emscripten::function("getVolumes", &bridge_get_volumes);
    emscripten::function("setVolume", &bridge_set_volume);
}
