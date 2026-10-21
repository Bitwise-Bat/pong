import { useState } from "react";
import useEvent from "../hooks/useEvent.ts";

const WARNING_SECONDS = 10;

export default () => {
    const [seconds, setSeconds] = useState<number | null>(null);

    useEvent("time", (e) => setSeconds(e.seconds));

    if (seconds === null) return null;

    const warning = seconds > 0 && seconds <= WARNING_SECONDS;
    const text = `${Math.floor(seconds / 60)}:${String(seconds % 60).padStart(2, "0")}`;

    return <div className={`absolute top-0 left-1/2 -translate-x-1/2 px-4 py-2 text-lg font-semibold tabular-nums bg-black/80 rounded-b ${warning ? "text-red-400 animate-pulse" : "text-white"}`}>
        {text}
    </div>
}
