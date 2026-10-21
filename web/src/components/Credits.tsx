interface Credit {
    name: string;
    role: string;
    url: string;
}

const SECTIONS: { title: string; credits: Credit[] }[] = [
    {
        title: "Libraries",
        credits: [
            { name: "SDL3",        role: "Platform layer",   url: "https://libsdl.org" },
            { name: "Dawn",        role: "WebGPU",           url: "https://dawn.googlesource.com/dawn" },
            { name: "dr_libs",     role: "MP3 decoding",     url: "https://github.com/mackron/dr_libs" },
            { name: "Emscripten",  role: "WebAssembly",      url: "https://emscripten.org" },
            { name: "React",       role: "UI",               url: "https://react.dev" },
            { name: "React Flow",  role: "Skill tree",       url: "https://reactflow.dev" },
            { name: "React Icons", role: "Icons",            url: "https://react-icons.github.io/react-icons" },
            { name: "Tailwind CSS", role: "Styling",         url: "https://tailwindcss.com" },
            { name: "Vite",        role: "Build tool",       url: "https://vite.dev" },
        ],
    },
    {
        title: "Assets",
        credits: [
            { name: "ElevenLabs", role: "Sounds", url: "https://elevenlabs.io" },
            { name: "Gemini",     role: "Images", url: "https://gemini.google.com" },
        ],
    },
];

interface CreditsProps {
    onClose: () => void;
}

export default ({ onClose }: CreditsProps) => {
    return <div className="absolute inset-0 flex items-center justify-center bg-slate-950/95">
        <div className="flex flex-col items-center gap-8 max-h-full overflow-y-auto py-10">
            <h2 className="text-2xl font-semibold tracking-[0.35em] pl-[0.35em] text-white">CREDITS</h2>
            {SECTIONS.map(({ title, credits }) => <section key={title} className="flex flex-col items-center gap-3">
                <h3 className="text-[11px] uppercase tracking-[0.4em] pl-[0.4em] text-cyan-300/70">{title}</h3>
                <ul className="grid grid-cols-[auto_auto] gap-x-6 gap-y-1 text-sm">
                    {credits.map(({ name, role, url }) => <li key={name} className="contents">
                        <a href={url} target="_blank" rel="noreferrer" className="text-right text-white hover:text-cyan-300 transition-colors">{name}</a>
                        <span className="text-white/45">{role}</span>
                    </li>)}
                </ul>
            </section>)}
        </div>
        <button
            onClick={onClose}
            className="absolute bottom-3 left-3 px-3 py-1 rounded-sm bg-black/60 text-sm text-white/70 hover:text-white cursor-pointer"
        >
            Back
        </button>
    </div>
}
