import { useState } from "react";
import { Handle, Position, ReactFlow, type BuiltInEdge, type Node, type NodeProps } from "@xyflow/react";
import "@xyflow/react/dist/style.css";
import type { MainModule } from "../wasm/pong";
import useEvent from "../hooks/useEvent.ts";
import { GiTwoCoins } from "react-icons/gi";
import { COIN_COLOR } from "../money.ts";

// Shape of the objects returned by the getSkills embind function (see web_bridge.cpp).
interface Skill {
    id: number;
    name: string;
    statName: string;
    multiply: boolean;
    x: number;
    y: number;
    parents: number[];
    level: number;
    maxLevel: number;
    values: number[];
    cost: number;
    canBuy: boolean;
}

const bonus = (skill: Skill, value: number) => `${skill.multiply ? "x" : value < 0 ? "" : "+"}${value} ${skill.statName}`;

type SkillNode = Node<{ skill: Skill }, "skill">;

const SkillCard = ({ data: { skill } }: NodeProps<SkillNode>) => {
    const maxed = skill.level >= skill.maxLevel;
    const owned = skill.level > 0;
    const total = skill.values.slice(0, skill.level).reduce((a, v) => skill.multiply ? a * v : a + v, skill.multiply ? 1 : 0);

    const state = maxed
        ? "border-cyan-300 bg-cyan-300/20"
        : skill.canBuy
            ? `${owned ? "border-cyan-300" : "border-white/60"} bg-slate-800 hover:bg-slate-700 cursor-pointer`
            : owned
                ? "border-cyan-300/60 bg-slate-900"
                : "border-white/15 bg-slate-900 opacity-50";

    return <div className={`w-40 px-3 py-2 rounded-sm border text-white text-center ${state}`}>
        <Handle type="target" position={Position.Top} style={{ opacity: 0 }}/>
        <div className="font-semibold">{skill.name}</div>
        {skill.maxLevel > 1 && <div className="my-1 flex justify-center gap-1">
            {skill.values.map((_, i) => <div key={i} className={`h-1.5 w-4 rounded-[1px] ${i < skill.level ? "bg-cyan-300" : "bg-white/20"}`}/>)}
        </div>}
        <div className="text-xs text-white/60">
            {maxed ? bonus(skill, total) : bonus(skill, skill.values[skill.level])}
            {owned && !maxed && <span className="text-white/35"> ({bonus(skill, total)})</span>}
        </div>
        {!maxed && <div className="mt-1 flex items-center justify-center gap-1 text-sm">
            <GiTwoCoins size={18} color={COIN_COLOR}/>
            {skill.cost}
        </div>}
        <Handle type="source" position={Position.Bottom} style={{ opacity: 0 }}/>
    </div>
}

const nodeTypes = { skill: SkillCard };

interface SkillsProps {
    game: MainModule;
    onClose: () => void;
}

export default ({ game, onClose }: SkillsProps) => {
    const [skills, setSkills] = useState<Skill[]>(() => game.getSkills());
    const refresh = () => setSkills(game.getSkills());

    useEvent("money", refresh);
    useEvent("skill", refresh);

    // A skill shows up only once all of its parents are owned.
    const visible = skills.filter((skill) => skill.parents.every((parent) => skills[parent].level > 0));

    const nodes: SkillNode[] = visible.map((skill) => ({
        id: String(skill.id),
        type: "skill",
        position: { x: skill.x, y: skill.y },
        data: { skill },
    }));

    const edges: BuiltInEdge[] = visible.flatMap((skill) => skill.parents.map((parent): BuiltInEdge => ({
        id: `${parent}-${skill.id}`,
        source: String(parent),
        target: String(skill.id),
        type: "smoothstep",
        pathOptions: { borderRadius: 4 },
        style: { stroke: skills[parent].level > 0 ? "rgb(103 232 249)" : "rgb(255 255 255 / 0.25)" },
    })));

    return <div className="absolute inset-0 bg-slate-950/95">
        <ReactFlow
            nodes={nodes}
            edges={edges}
            nodeTypes={nodeTypes}
            colorMode="dark"
            fitView
            fitViewOptions={{ maxZoom: 1 }}
            proOptions={{ hideAttribution: true }}
            nodesDraggable={false}
            nodesConnectable={false}
            elementsSelectable={false}
            onNodeClick={(_, node) => game.buySkill(Number(node.id))}
        />
        <button
            onClick={onClose}
            className="absolute bottom-3 left-3 px-3 py-1 rounded-sm bg-black/60 text-sm text-white/70 hover:text-white cursor-pointer"
        >
            Back
        </button>
    </div>
}
