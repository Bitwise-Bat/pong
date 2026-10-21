#include "save.hpp"

#include <algorithm>
#include <string>

#include <emscripten/html5.h>
#include <emscripten/val.h>

#include "events.hpp"
#include "game.hpp"
#include "skills.hpp"

namespace pong
{
    namespace
    {
        using emscripten::val;

        constexpr const char* SAVE_KEY = "pong.save";
        constexpr int SAVE_VERSION = 1;
        constexpr size_t MATERIAL_COUNT = static_cast<size_t>(Material::Count);

        const char* on_before_unload(int, const void*, void* user_data)
        {
            static_cast<const Save*>(user_data)->save();
            return nullptr;
        }
    }

    void Save::init(Game& game)
    {
        m_game = &game;
        load();

        m_skill_sub = game.events().subscribe<SkillBought>([this](const SkillBought&) { save(); });
        m_round_sub = game.events().subscribe<GameOver>([this](const GameOver&) { save(); });
        emscripten_set_beforeunload_callback(this, &on_before_unload);
    }

    // Skills are stored by name, so adding or reordering skills keeps old saves valid.
    void Save::save() const
    {
        val ores = val::array();
        for (size_t i = 0; i < MATERIAL_COUNT; ++i)
        {
            ores.call<void>("push", m_game->ore(i));
        }

        val skills = val::object();
        for (size_t i = 0; i < SKILLS.size(); ++i)
        {
            const uint8_t level = m_game->skill_level(i);
            if (level > 0) skills.set(std::string(SKILLS[i].name), level);
        }

        val data = val::object();
        data.set("version", SAVE_VERSION);
        data.set("ores", ores);
        data.set("skills", skills);

        const auto json = val::global("JSON").call<std::string>("stringify", data);
        val::global("localStorage").call<void>("setItem", std::string(SAVE_KEY), json);
    }

    void Save::load()
    {
        const val item = val::global("localStorage").call<val>("getItem", std::string(SAVE_KEY));
        if (item.isNull()) return;

        const val data = val::global("JSON").call<val>("parse", item);
        if (data.isNull() || data.typeOf().as<std::string>() != "object") return;

        const val version = data["version"];
        if (!version.isNumber() || version.as<int>() != SAVE_VERSION) return;

        const val ores = data["ores"];
        if (ores.isArray())
        {
            const size_t count = std::min<size_t>(ores["length"].as<uint32_t>(), MATERIAL_COUNT);
            for (size_t i = 0; i < count; ++i)
            {
                const val ore = ores[static_cast<int>(i)];
                if (ore.isNumber()) m_game->set_ore(i, ore.as<uint32_t>());
            }
        }

        const val skills = data["skills"];
        if (skills.isNull() || skills.typeOf().as<std::string>() != "object") return;

        for (size_t i = 0; i < SKILLS.size(); ++i)
        {
            const val level = skills[std::string(SKILLS[i].name)];
            if (level.isNumber()) m_game->set_skill_level(i, level.as<uint32_t>());
        }
    }
}
