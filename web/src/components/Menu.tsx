import { useEffect, useState } from "react";
import type {MainModule} from "../wasm/pong";
import useEvent from "../hooks/useEvent.ts";
import Skills from "./Skills.tsx";
import Credits from "./Credits.tsx";
import Settings from "./Settings.tsx";

interface MenuProps {
    game: MainModule
}

export default ({ game } : MenuProps) => {
    const [open, setOpen] = useState(true);
    const [shown, setShown] = useState(false);
    const [gameOver, setGameOver] = useState(false);
    const [timeUp, setTimeUp] = useState(false);
    const [skillsOpen, setSkillsOpen] = useState(false);
    const [creditsOpen, setCreditsOpen] = useState(false);
    const [settingsOpen, setSettingsOpen] = useState(false);

    useEvent("gameOver", (e) => {
        setGameOver(true);
        setTimeUp(e.reason === "time");
        setOpen(true);
    });

    useEffect(() => {
        const id = requestAnimationFrame(() => setShown(true));
        return () => cancelAnimationFrame(id);
    }, []);

    if (!open) return null;

    const start = () => {
        game.startGame();
        setOpen(false);
    };

    return <div className={`absolute inset-0 flex items-center justify-center bg-slate-950/40 backdrop-blur-[2px] transition-opacity duration-500 ${shown ? "opacity-100" : "opacity-0"}`}>
        <div className={`flex flex-col items-center gap-9 px-14 py-11 rounded-2xl border border-white/10 bg-slate-900/60 shadow-2xl shadow-black/40 backdrop-blur-md transition-all duration-700 ${shown ? "translate-y-0 opacity-100" : "translate-y-3 opacity-0"}`}>
            <div className="flex flex-col items-center gap-3">
                <h1 className="text-5xl font-semibold tracking-[0.35em] pl-[0.35em] text-white">{gameOver ? (timeUp ? "TIME'S UP" : "GAME OVER") : "PONG"}</h1>
                <div className="h-px w-16 bg-gradient-to-r from-transparent via-cyan-300/70 to-transparent" />
                <p className="text-[11px] uppercase tracking-[0.4em] pl-[0.4em] text-white/45">Break. Mine. Collect.</p>
            </div>
            <button
                autoFocus
                onClick={start}
                className="px-10 py-2.5 rounded-full border border-cyan-300/40 bg-cyan-300/10 text-sm font-medium uppercase tracking-[0.25em] pl-[calc(2.5rem+0.25em)] text-cyan-100 transition duration-200 hover:bg-cyan-300 hover:text-slate-950 hover:shadow-lg hover:shadow-cyan-300/30 focus-visible:outline-2 focus-visible:outline-offset-4 focus-visible:outline-cyan-300 active:scale-95 cursor-pointer"
            >
                {gameOver ? "Play again" : "Start"}
            </button>
            <button
                onClick={() => setSkillsOpen(true)}
                className="-mt-5 text-xs uppercase tracking-[0.3em] pl-[0.3em] text-white/50 hover:text-white transition-colors cursor-pointer"
            >
                Skills
            </button>
            <button
                onClick={() => setCreditsOpen(true)}
                className="-mt-7 text-xs uppercase tracking-[0.3em] pl-[0.3em] text-white/50 hover:text-white transition-colors cursor-pointer"
            >
                Credits
            </button>
            <button
                onClick={() => setSettingsOpen(true)}
                className="-mt-7 text-xs uppercase tracking-[0.3em] pl-[0.3em] text-white/50 hover:text-white transition-colors cursor-pointer"
            >
                Settings
            </button>
        </div>
        {skillsOpen && <Skills game={game} onClose={() => setSkillsOpen(false)}/>}
        {creditsOpen && <Credits onClose={() => setCreditsOpen(false)}/>}
        {settingsOpen && <Settings game={game} onClose={() => setSettingsOpen(false)}/>}
    </div>
}
