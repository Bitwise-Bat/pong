export type PongEvent =
    | { type: "life";      lives: number }
    | { type: "ore";       material: number; count: number }
    | { type: "buffStart"; buff: number; duration: number }
    | { type: "buffEnd";   buff: number }
    | { type: "gameOver" };

export type EventType = PongEvent["type"];
export type EventOf<K extends EventType> = Extract<PongEvent, { type: K }>;

const listeners = new Map<EventType, Set<(e: PongEvent) => void>>();

export function emit(e: PongEvent) {
    listeners.get(e.type)?.forEach((h) => h(e));
}

export function on<K extends EventType>(type: K, handler: (e: EventOf<K>) => void) {
    let set = listeners.get(type);
    if (!set) listeners.set(type, (set = new Set()));
    const h = handler as (e: PongEvent) => void;
    set.add(h);
    return () => { set!.delete(h); };
}

export function off<K extends EventType>(type: K, handler: (EventType : EventOf<K>) => void){
    let set = listeners.get(type);
    if (!set) return;
    set.delete(handler as (e: PongEvent) => void);
}
