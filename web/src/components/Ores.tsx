import { useState } from "react";
import type { IconType } from "react-icons";
import { GiDiamondHard, GiGoldBar, GiMetalBar, GiStoneBlock } from "react-icons/gi";
import useEvent from "../hooks/useEvent.ts";

// Enum order of Material in box.hpp, colors match material_color in game.cpp.
const ORES: { name: string; Icon: IconType; color: string }[] = [
    { name: "Stone",   Icon: GiStoneBlock,   color: "rgb(140 140 140)" },
    { name: "Iron",    Icon: GiMetalBar,     color: "rgb(190 195 205)" },
    { name: "Bronze",  Icon: GiMetalBar,     color: "rgb(190 120 60)" },
    { name: "Silver",  Icon: GiMetalBar,     color: "rgb(225 230 240)" },
    { name: "Gold",    Icon: GiGoldBar,      color: "rgb(250 205 60)" },
    { name: "Diamond", Icon: GiDiamondHard,  color: "rgb(110 230 245)" },
];

export default () => {
    const [ores, setOres] = useState<number[]>(() => ORES.map(() => 0));

    useEvent("ore", (e) => setOres((prev) => prev.map((c, i) => (i === e.material ? e.count : c))));

    return <ul className="absolute top-0 left-0 flex gap-4 px-3 py-2 text-white bg-black/80 rounded-br">
        {ORES.map(({ name, Icon, color }, i) => <li key={name} title={name} className="flex items-center gap-1">
            <Icon size={22} color={color}/>
            {ores[i]}
        </li>)}
    </ul>
}
