#include "save.hpp"

#include <algorithm>
#include <array>
#include <utility>
#include <string>

#include <emscripten/html5.h>
#include <emscripten/val.h>

#include "audio.hpp"
#include "events.hpp"
#include "game.hpp"
#include "skills.hpp"

namespace pong
{
    namespace
    {
        using emscripten::val;

        constexpr const char* SAVE_KEY = "pong.save";
        // Version 1 stored a count per ore instead of money.
        constexpr int SAVE_VERSION = 2;
        constexpr size_t MATERIAL_COUNT = static_cast<size_t>(Material::Count);
        constexpr std::array<std::pair<const char*, Volume>, 3> VOLUME_KEYS{{
            { "main", Volume::Main }, { "music", Volume::Music }, { "sfx", Volume::Sfx },
        }};

        const char* on_before_unload(int, const void*, void* user_data)
        {
            static_cast<const Save*>(user_data)->save();
            return nullptr;
        }
    }

    void Save::init(Game& game, Audio& audio)
    {
        m_game = &game;
        m_audio = &audio;
        load();

        m_skill_sub = game.events().subscribe<SkillBought>([this](const SkillBought&) { save(); });
        m_round_sub = game.events().subscribe<GameOver>([this](const GameOver&) { save(); });
        emscripten_set_beforeunload_callback(this, &on_before_unload);
    }

    // Skills are stored by name, so adding or reordering skills keeps old saves valid.
    void Save::save() const
    {
        val skills = val::object();
        for (size_t i = 0; i < SKILLS.size(); ++i)
        {
            const uint8_t level = m_game->skill_level(i);
            if (level > 0) skills.set(std::string(SKILLS[i].name), level);
        }

        val volume = val::object();
        for (const auto& [key, channel] : VOLUME_KEYS)
        {
            volume.set(key, m_audio->volume(channel));
        }

        val data = val::object();
        data.set("version", SAVE_VERSION);
        data.set("money", m_game->money());
        data.set("skills", skills);
        data.set("volume", volume);

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
        if (!version.isNumber()) return;

        if (version.as<int>() == SAVE_VERSION)
        {
            const val money = data["money"];
            if (money.isNumber()) m_game->set_money(money.as<uint32_t>());
        }
        else if (version.as<int>() == 1)
        {
            // Converts the old ores to money at their material value.
            const val ores = data["ores"];
            if (ores.isArray())
            {
                uint32_t money = 0;
                const size_t count = std::min<size_t>(ores["length"].as<uint32_t>(), MATERIAL_COUNT);
                for (size_t i = 0; i < count; ++i)
                {
                    const val ore = ores[static_cast<int>(i)];
                    if (ore.isNumber()) money += ore.as<uint32_t>() * material_value(static_cast<Material>(i));
                }
                m_game->set_money(money);
            }
        }
        else
        {
            return;
        }

        const val volume = data["volume"];
        if (!volume.isNull() && volume.typeOf().as<std::string>() == "object")
        {
            for (const auto& [key, channel] : VOLUME_KEYS)
            {
                const val value = volume[key];
                if (value.isNumber()) m_audio->set_volume(channel, value.as<float>());
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
