import { useState } from "react";
import useEvent from "../hooks/useEvent.ts";

const NAMES = ["Stone", "Iron", "Bronze", "Silver", "Gold", "Diamond"];

export default () => {
    const [ores, setOres] = useState<number[]>(() => NAMES.map(() => 0));

    useEvent("ore", (e) => setOres((prev) => prev.map((c, i) => (i === e.material ? e.count : c))));

    return <ul className="absolute top-0 left-0 px-3 py-2 text-white bg-black/80 rounded-br">
        {NAMES.map((name, i) => <li key={name}>{name}: {ores[i]}</li>)}
    </ul>
}
