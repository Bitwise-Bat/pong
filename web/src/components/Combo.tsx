import { useState } from "react";
import useEvent from "../hooks/useEvent.ts";

export default () => {
    const [multiplier, setMultiplier] = useState(1);

    useEvent("combo", (e) => setMultiplier(e.multiplier));

    if (multiplier < 2) return null;

    return <div className="absolute top-12 left-1/2 -translate-x-1/2 px-3 py-1 bg-black/60 rounded text-sm font-semibold tabular-nums text-white">
        COMBO <span className="font-bold text-amber-300">x{multiplier}</span>
    </div>
}
