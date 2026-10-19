import createPong, {type MainModule} from "../wasm/pong.js"
import {useEffect, useState} from "react";

export default () => {
    const [isReady, setReady] = useState(false);
    const [game, setGame] = useState<MainModule | null>(null);
    
    useEffect(() => {
        createPong().then(mo => {
            setGame(mo);
            setReady(true);
        }).catch((err) => {
            console.error(err);
        });
    }, [])
    
    return {
        game,  isReady
    }
}