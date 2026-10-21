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
        </div>
        <button
            onClick={onClose}
            className="absolute bottom-3 left-3 px-3 py-1 rounded-sm bg-black/60 text-sm text-white/70 hover:text-white cursor-pointer"
        >
            Back
        </button>
    </div>
}
