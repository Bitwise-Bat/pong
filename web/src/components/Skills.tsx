import { useState } from "react";
import { Handle, Position, ReactFlow, type BuiltInEdge, type Node, type NodeProps } from "@xyflow/react";
import "@xyflow/react/dist/style.css";
import type { MainModule } from "../wasm/pong";
import useEvent from "../hooks/useEvent.ts";
import { ORES } from "../ores.ts";

// Shape of the objects returned by the getSkills embind function (see web_bridge.cpp).
interface Skill {
    id: number;
    name: string;
    description: string;
    x: number;
    y: number;
    parents: number[];
    costMaterial: number;
    cost: number;
    owned: boolean;
    canBuy: boolean;
}

type SkillNode = Node<{ skill: Skill }, "skill">;

const SkillCard = ({ data: { skill } }: NodeProps<SkillNode>) => {
    const { Icon, color } = ORES[skill.costMaterial];
    const state = skill.owned
        ? "border-cyan-300 bg-cyan-300/20"
        : skill.canBuy
            ? "border-white/60 bg-slate-800 hover:bg-slate-700 cursor-pointer"
            : "border-white/15 bg-slate-900 opacity-50";

    return <div className={`w-40 px-3 py-2 rounded-sm border text-white text-center ${state}`}>
        <Handle type="target" position={Position.Top} style={{ opacity: 0 }}/>
        <div className="font-semibold">{skill.name}</div>
        <div className="text-xs text-white/60">{skill.description}</div>
        {!skill.owned && <div className="mt-1 flex items-center justify-center gap-1 text-sm">
            <Icon size={18} color={color}/>
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

    useEvent("ore", refresh);
    useEvent("skill", refresh);

    const nodes: SkillNode[] = skills.map((skill) => ({
        id: String(skill.id),
        type: "skill",
        position: { x: skill.x, y: skill.y },
        data: { skill },
    }));

    const edges: BuiltInEdge[] = skills.flatMap((skill) => skill.parents.map((parent): BuiltInEdge => ({
        id: `${parent}-${skill.id}`,
        source: String(parent),
        target: String(skill.id),
        type: "smoothstep",
        pathOptions: { borderRadius: 4 },
        style: { stroke: skills[parent].owned ? "rgb(103 232 249)" : "rgb(255 255 255 / 0.25)" },
    })));

    return <div className="absolute inset-0 bg-slate-950/95">
        <ReactFlow
            nodes={nodes}
            edges={edges}
            nodeTypes={nodeTypes}
            colorMode="dark"
            fitView
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
