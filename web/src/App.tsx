import useGame from "./hooks/useGame.ts";

function App() {
  const { game, isReady } = useGame();
  
  if (!isReady) {
    return <>Loading...</>
  }
  
  return (
    <div className="flex w-screen h-screen items-center justify-center bg-black">
      <div className="relative w-[1080px] h-[720px] overflow-hidden bg-gray-800 border border-gray-400 rounded">
        <canvas />
      </div>
    </div>
  )
}

export default App
