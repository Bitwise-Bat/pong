# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Overview

A breakout-style pong game written in C++23 that only builds for WebAssembly (the CMake configure step fails with `FATAL_ERROR` outside Emscripten). The C++ core renders through SDL3 into a canvas, and a React + Vite frontend in `web/` hosts the canvas and draws UI on top of it. There are no tests.

## Commands

C++ (requires `VCPKG_ROOT` and `EMSDK` environment variables, Ninja, and an activated emsdk):

```
cmake --preset wasm-dev          # or wasm-prod
cmake --build --preset wasm-dev
```

The build writes `pong.js`, `pong.wasm` and the generated `pong.d.ts` into `web/src/wasm/` (git-ignored). The web app cannot typecheck or run until the wasm module has been built at least once.

Frontend (run from `web/`):

```
npm run dev      # vite dev server
npm run build    # tsc -b && vite build
npm run lint     # oxlint
```

## Architecture

- `main.cpp` uses the SDL3 main-callbacks API (`SDL_AppInit` / `SDL_AppIterate` / ...). All state lives in `AppContext` (`source/application.hpp`): window, renderer and `pong::Game`.
- `pong::Game` (`source/game.*`) owns the simulation: paddle, balls, boxes, score. `update()` runs paddle -> balls (walls, paddle) -> ball/box collisions -> ball/ball collisions, then flushes the event bus.
- Rendering goes through the abstract `Renderer` (`source/renderer/renderer.hpp`); `SdlRenderer` is the only implementation. Game code draws via `fill_rect` with the types from `math.hpp`.
- Ball behavior is data-driven: `Ball::base` holds `Stats`, and `modifiers` (add, then multiply; timed or permanent via `remaining == -1`) are folded together by `Ball::stats()`. Always read speed, size and damage from `stats()`, never from `base`. `hitbox()` derives from the `Size` stat.
- `collision.hpp::overlap` returns the minimum-translation vector for the first rect; callers apply it and reflect velocity along that axis.

### C++ to React event flow

`Game` publishes typed events (`source/events.hpp`) on `EventBus`. Publishing is deferred: events queue up and are delivered on `flush()` at the end of `Game::update`. `web_bridge.cpp` subscribes to them in `bridge_attach` and forwards each one as a plain JS object (`{ type, ... }`) to a callback registered from JS through the embind functions `onEvent` / `clearEvent`.

On the JS side `useGame` loads the module and calls `mo.onEvent(emit)`. `web/src/events.ts` holds the `PongEvent` union and a small pub/sub; components subscribe with the `useEvent` hook. When adding an event, update `events.hpp`, `web_bridge.cpp` and the `PongEvent` union in `events.ts` together (some union members there, such as `life` and `buffStart`, have no C++ producer yet).

## Conventions

- C++: 4-space indent, Allman braces, `m_` prefix for members, `snake_case` functions, everything in `namespace pong`. New `.cpp` files must be added to `target_sources` in `CMakeLists.txt`.
- Code and comments are English only; add comments only when they explain something non-obvious.
- Every file ends with a trailing newline.

## Scope

- Do only what was asked. No unrelated refactors, renames, reformatting or extra features.
- If you notice a bug or cleanup opportunity outside the request, mention it and leave the code as is.
- Ask before changing behavior that was not part of the request.

## Commits

- Write messages in English using Conventional Commits: `type(scope): summary` with types such as `feat`, `fix`, `refactor`, `style`, `docs`, `chore`. The scope is optional (`game`, `renderer`, `wasm`, `web`).
- Summary in the imperative mood, lowercase, no trailing period. Add a body only when the reason is not obvious from the diff.
- Never add AI attribution: no `Co-Authored-By` trailer and no "Generated with" line, in commits or PR descriptions.
- Keep commits small and logical: one concern per commit, and each commit should build on its own. Stage files explicitly; do not leave unrelated changes (such as debug tweaks) in a commit.
- Commit only when asked.
