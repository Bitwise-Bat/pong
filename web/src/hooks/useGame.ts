import createPong, {type MainModule} from "../wasm/pong.js"
import {useEffect, useRef, useState} from "react";
import {emit, type PongEvent} from "../events.ts";

let gamePromise : Promise<MainModule> | null = null;

export default () => {
    const [game, setGame] = useState<MainModule | null>(null);
    const canvasRef = useRef<HTMLCanvasElement>(null);
    
    useEffect(() => {
        if (!canvasRef.current) return;
        
        let active = true;
        
        gamePromise ??= createPong({
            canvas: canvasRef.current,
            requestFullscreen: undefined
        });

        gamePromise.then(mo => {
            if (!active) return;

            setGame(mo);
            mo.onEvent((event: PongEvent) => emit(event));
        }).catch((err) => {
            console.error(err);
        });

        return () => {
            active = false;
        }
    }, [])
    
    return {
        game, isReady: game !== null, canvasRef
    }
}
