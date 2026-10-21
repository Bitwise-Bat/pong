import { useEffect, useRef } from "react";
import useGame from "./hooks/useGame.ts";
import Overlay from "./components/Overlay.tsx";

function App() {
  const { canvasRef, game } = useGame();
  const rootRef = useRef<HTMLDivElement>(null);

  const toggleFullscreen = () => {
    if (document.fullscreenElement) void document.exitFullscreen();
    else void rootRef.current?.requestFullscreen();
  };

  useEffect(() => {
    const onKey = (e: KeyboardEvent) => {
      if (e.code === "KeyF" && !e.repeat) toggleFullscreen();
    };
    window.addEventListener("keydown", onKey);
    return () => window.removeEventListener("keydown", onKey);
  }, []);

  return (
    <div ref={rootRef} className="flex w-screen h-screen items-center justify-center bg-black">
      <div className="relative aspect-video w-[min(100vw,calc(100vh*16/9))] overflow-hidden bg-gray-800">
        <canvas ref={canvasRef} id="canvas" className="absolute left-0 top-0 w-full h-full" />
        { game && <Overlay game={game}/>}
        <button
          onClick={toggleFullscreen}
          title="Fullscreen (F)"
          className="absolute top-3 right-3 px-2 py-1 rounded bg-black/60 text-xs text-white/70 hover:text-white cursor-pointer"
        >
          ⛶
        </button>
      </div>
    </div>
  )
}

export default App
