import useGame from "./hooks/useGame.ts";
import Overlay from "./components/Overlay.tsx";

function App() {
  const { canvasRef, game } = useGame();
  
  return (
    <div className="flex w-screen h-screen items-center justify-center bg-black">
      <div className="relative w-[800px] h-[600px] overflow-hidden bg-gray-800 border border-gray-400 rounded">
        <canvas ref={canvasRef} id="canvas" className="absolute left-0 top-0 w-full h-full" />
          { game && <Overlay game={game}/>}
      </div>
    </div>
  )
}

export default App
