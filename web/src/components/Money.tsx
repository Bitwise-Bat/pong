import { useState } from "react";
import { GiTwoCoins } from "react-icons/gi";
import useEvent from "../hooks/useEvent.ts";
import { COIN_COLOR } from "../money.ts";

export default () => {
    const [money, setMoney] = useState(0);

    useEvent("money", (e) => setMoney(e.amount));

    return <div className="absolute top-0 left-0 flex items-center gap-1.5 px-3 py-2 text-white tabular-nums bg-black/80 rounded-br">
        <GiTwoCoins size={22} color={COIN_COLOR}/>
        {money}
    </div>
}
