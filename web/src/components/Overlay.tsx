import type {MainModule} from "../wasm/pong";
import useEvent from "../hooks/useEvent.ts";
import Menu from "./Menu.tsx";
import Ores from "./Ores.tsx";

interface OverlayProps {
    game: MainModule
}

export default ({ game } : OverlayProps) => {
    
    
    return <div className="absolute top-0 left-0 w-full h-full p-5">
        <Menu/>
        <Ores/>
    </div>
}
