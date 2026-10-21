import { useState } from "react";
import type { MainModule } from "../wasm/pong";

// Order matches pong::Volume in audio.hpp.
const CHANNELS = [
    { key: "main",  label: "Main" },
    { key: "music", label: "Music" },
    { key: "sfx",   label: "SFX" },
] as const;

type Volumes = Record<(typeof CHANNELS)[number]["key"], number>;

interface SettingsProps {
    game: MainModule;
    onClose: () => void;
}

export default ({ game, onClose }: SettingsProps) => {
    const [volumes, setVolumes] = useState<Volumes>(() => game.getVolumes());
    const [confirmReset, setConfirmReset] = useState(false);

    const reset = () => {
        game.resetGame();
        setVolumes(game.getVolumes());
        setConfirmReset(false);
    };

    const change = (index: number, value: number) => {
        game.setVolume(index, value);
        setVolumes((prev) => ({ ...prev, [CHANNELS[index].key]: value }));
    };

    return <div className="absolute inset-0 flex items-center justify-center bg-slate-950/95">
        <div className="flex flex-col items-center gap-8">
            <h2 className="text-2xl font-semibold tracking-[0.35em] pl-[0.35em] text-white">SETTINGS</h2>
            <section className="flex flex-col items-center gap-3">
                <h3 className="text-[11px] uppercase tracking-[0.4em] pl-[0.4em] text-cyan-300/70">Volume</h3>
                <div className="grid grid-cols-[auto_12rem_2.5rem] items-center gap-x-4 gap-y-3 text-sm">
                    {CHANNELS.map(({ key, label }, i) => <label key={key} className="contents">
                        <span className="text-right text-white">{label}</span>
                        <input
                            type="range"
                            min={0}
                            max={100}
                            value={Math.round(volumes[key] * 100)}
                            onChange={(e) => change(i, Number(e.target.value) / 100)}
                            className="accent-cyan-300 cursor-pointer"
                        />
                        <span className="text-right tabular-nums text-white/45">{Math.round(volumes[key] * 100)}</span>
                    </label>)}
                </div>
            </section>
            <button
                onClick={() => setConfirmReset(true)}
                className="px-4 py-1.5 rounded-sm border border-red-400/50 text-xs uppercase tracking-[0.25em] pl-[calc(1rem+0.25em)] text-red-300 hover:bg-red-400 hover:text-slate-950 transition-colors cursor-pointer"
            >
                Reset game
            </button>
        </div>
        {confirmReset && <div className="absolute inset-0 z-10 flex items-center justify-center bg-black/70">
            <div role="dialog" aria-modal="true" className="flex flex-col items-center gap-5 max-w-sm px-8 py-6 rounded-sm border border-white/10 bg-slate-900 text-center">
                <h3 className="text-lg font-semibold text-white">Reset game?</h3>
                <p className="text-sm text-white/60">This deletes your money, skills and settings. It cannot be undone.</p>
                <div className="flex gap-3">
                    <button
                        autoFocus
                        onClick={() => setConfirmReset(false)}
                        className="px-4 py-1.5 rounded-sm bg-white/10 text-sm text-white hover:bg-white/20 cursor-pointer"
                    >
                        Cancel
                    </button>
                    <button
                        onClick={reset}
                        className="px-4 py-1.5 rounded-sm bg-red-500 text-sm font-medium text-white hover:bg-red-400 cursor-pointer"
                    >
                        Reset
                    </button>
                </div>
            </div>
        </div>}
        <button
            onClick={onClose}
            className="absolute bottom-3 left-3 px-3 py-1 rounded-sm bg-black/60 text-sm text-white/70 hover:text-white cursor-pointer"
        >
            Back
        </button>
    </div>
}
