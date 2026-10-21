import type {MainModule} from "../wasm/pong";
import Menu from "./Menu.tsx";
import Ores from "./Ores.tsx";
import Timer from "./Timer.tsx";
import Combo from "./Combo.tsx";

interface OverlayProps {
    game: MainModule
}

export default ({ game } : OverlayProps) => {
    
    
    return <div className="absolute top-0 left-0 w-full h-full p-5">
        <Menu game={game}/>
        <Ores/>
        <Timer/>
        <Combo/>
    </div>
}
