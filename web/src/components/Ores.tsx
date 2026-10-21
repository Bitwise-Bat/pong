import { useState } from "react";
import useEvent from "../hooks/useEvent.ts";
import { ORES } from "../ores.ts";

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
