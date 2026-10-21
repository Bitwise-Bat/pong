import type {MainModule} from "../wasm/pong";
import Menu from "./Menu.tsx";
import Money from "./Money.tsx";
import Timer from "./Timer.tsx";
import Combo from "./Combo.tsx";

interface OverlayProps {
    game: MainModule
}

export default ({ game } : OverlayProps) => {
    const movePointer = (e: React.PointerEvent<HTMLDivElement>) => {
        const rect = e.currentTarget.getBoundingClientRect();
        game.setPointer((e.clientX - rect.left) / rect.width, (e.clientY - rect.top) / rect.height);
    };

    return <div
        className="absolute top-0 left-0 w-full h-full p-5"
        onPointerMove={movePointer}
        onPointerLeave={() => game.setPointer(-1, -1)}
    >
        <Menu game={game}/>
        <Money/>
        <Timer/>
        <Combo/>
    </div>
}
