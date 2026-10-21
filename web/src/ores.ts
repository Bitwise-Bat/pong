import type { IconType } from "react-icons";
import { GiDiamondHard, GiGoldBar, GiMetalBar, GiStoneBlock } from "react-icons/gi";

// Enum order of Material in box.hpp, colors match material_color in game.cpp.
export const ORES: { name: string; Icon: IconType; color: string }[] = [
    { name: "Stone",   Icon: GiStoneBlock,   color: "rgb(140 140 140)" },
    { name: "Iron",    Icon: GiMetalBar,     color: "rgb(190 195 205)" },
    { name: "Bronze",  Icon: GiMetalBar,     color: "rgb(190 120 60)" },
    { name: "Silver",  Icon: GiMetalBar,     color: "rgb(225 230 240)" },
    { name: "Gold",    Icon: GiGoldBar,      color: "rgb(250 205 60)" },
    { name: "Diamond", Icon: GiDiamondHard,  color: "rgb(110 230 245)" },
];
