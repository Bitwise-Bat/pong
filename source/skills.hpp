#pragma once
#include <array>
#include <cstdint>
#include <string_view>

#include "modifier.hpp"

namespace pong
{
    inline constexpr size_t MAX_SKILL_LEVEL = 5;

    struct SkillLevel
    {
        uint32_t cost{}; // money
        float value{};
    };

    struct SkillDef
    {
        std::string_view name;
        std::string_view stat_name; // shown after the bonus, e.g. "+2 damage"
        float x, y; // position in the skill tree view
        std::array<int8_t, 2> parents; // skill ids that must be owned first, -1 marks an unused slot
        Stat stat;
        Op op;
        std::array<SkillLevel, MAX_SKILL_LEVEL> levels; // bought in order, a zero cost ends the list

        [[nodiscard]] constexpr size_t max_level() const
        {
            size_t n = 0;
            while (n < levels.size() && levels[n].cost > 0) ++n;
            return n;
        }
    };

    inline constexpr std::array<SkillDef, 11> SKILLS{
        {
            {
                "Sharp Ball", "damage", 0.0f, 0.0f, {-1, -1}, Stat::Damage, Op::Add,
                {{{25, 1}, {50, 2}, {100, 3}}}
            },
            {
                "Heavy Ball", "size", -200.0f, 150.0f, {0, -1}, Stat::Size, Op::Add,
                {{{25, 4}, {50, 4}, {100, 6}}}
            },
            {
                "Slow Ball", "speed", 200.0f, 150.0f, {0, -1}, Stat::Speed, Op::Add,
                {{{30, -30}, {60, -30}, {120, -40}}}
            },
            {
                "Keen Edge", "damage", -200.0f, 300.0f, {1, -1}, Stat::Damage, Op::Add,
                {{{60, 2}, {120, 3}, {240, 4}}}
            },
            {
                "Fleet Ball", "speed", 200.0f, 300.0f, {2, -1}, Stat::Speed, Op::Add,
                {{{120, 60}, {240, 80}}}
            },
            {
                "Diamond Tip", "damage", 0.0f, 450.0f, {3, 4}, Stat::Damage, Op::Add,
                {{{250, 4}}}
            },
            {
                "Extra Time", "seconds", 0.0f, 150.0f, {0, -1}, Stat::Time, Op::Add,
                {{{45, 5}, {90, 5}, {180, 10}}}
            },
            {
                "Swift Paddle", "paddle speed", 400.0f, 150.0f, {0, -1}, Stat::PaddleSpeed, Op::Add,
                {{{45, 30}, {90, 30}, {180, 40}}}
            },
            {
                "Sprint", "sprint speed", 400.0f, 300.0f, {7, -1}, Stat::Sprint, Op::Add,
                {{{60, 100}, {120, 50}, {240, 50}}}
            },
            {
                "Streak", "max multiplier", -400.0f, 150.0f, {0, -1}, Stat::ComboCap, Op::Add,
                {{{30, 1}, {60, 2}, {120, 3}, {240, 3}}}
            },
            {
                "Grabby Hands", "cursor reach", -400.0f, 300.0f, {9, -1}, Stat::CursorReach, Op::Add,
                {{{120, 8}, {240, 8}, {480, 12}}}
            },
        }
    };
}
