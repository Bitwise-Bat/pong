#pragma once
#include <array>
#include <cstdint>
#include <string_view>

#include "box.hpp"
#include "modifier.hpp"

namespace pong
{
    struct SkillDef
    {
        std::string_view name;
        std::string_view description;
        float x, y; // position in the skill tree view
        std::array<int8_t, 2> parents; // skill ids that must be owned first, -1 marks an unused slot
        Material cost_material;
        uint32_t cost;
        Modifier effect; // folded into the base stats of the starting ball
    };

    inline constexpr std::array<SkillDef, 6> SKILLS{{
        { "Sharp Ball",  "+1 damage",   0.0f,    0.0f,   { -1, -1 }, Material::Stone,   10,
          { .stat = Stat::Damage, .op = Op::Add, .value = 1 } },
        { "Heavy Ball",  "Bigger ball", -200.0f, 150.0f, {  0, -1 }, Material::Stone,   25,
          { .stat = Stat::Size,   .op = Op::Add, .value = 4 } },
        { "Quick Ball",  "+40 speed",   200.0f,  150.0f, {  0, -1 }, Material::Iron,    10,
          { .stat = Stat::Speed,  .op = Op::Add, .value = 40 } },
        { "Keen Edge",   "+2 damage",   -200.0f, 300.0f, {  1, -1 }, Material::Bronze,  10,
          { .stat = Stat::Damage, .op = Op::Add, .value = 2 } },
        { "Fleet Ball",  "+60 speed",   200.0f,  300.0f, {  2, -1 }, Material::Silver,  10,
          { .stat = Stat::Speed,  .op = Op::Add, .value = 60 } },
        { "Diamond Tip", "+4 damage",   0.0f,    450.0f, {  3,  4 }, Material::Diamond, 5,
          { .stat = Stat::Damage, .op = Op::Add, .value = 4 } },
    }};
}
