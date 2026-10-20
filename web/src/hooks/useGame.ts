import createPong, {type MainModule} from "../wasm/pong.js"
import {useEffect, useRef, useState} from "react";

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
            if (active) setGame(mo);
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