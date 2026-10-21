import { useEffect, useRef } from "react";
import { on, type EventType, type EventOf } from "../events";

export default function useEvent<K extends EventType>(
    event: K,
    handler: (e: EventOf<K>) => void
) {
    const ref = useRef(handler);
    ref.current = handler;

    useEffect(() => on(event, (e) => ref.current(e)), [event]);
}
