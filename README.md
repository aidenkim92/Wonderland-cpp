# Wonderland (C++)

Bright pixel adventure remake. Design: [`docs/design-bible-v0.md`](docs/design-bible-v0.md).

Legacy Unity reference only: [aidenkim92/Wonderland](https://github.com/aidenkim92/Wonderland).

## Dependencies (macOS)

```bash
brew install cmake raylib
```

## Build & run

```bash
export DEVELOPER_DIR=/Applications/Xcode.app/Contents/Developer
export SDKROOT="$(xcrun --sdk macosx --show-sdk-path)"
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
./build/wonderland
```

If linking fails with `arm64e.x1` / malformed `.tbd` errors, the Command Line Tools SDK is ahead of your linker — the `DEVELOPER_DIR` / `SDKROOT` exports above fix that by using Xcode’s SDK.

## Controls (slice 6)

- **A / D** or arrows — move
- **Space / W / ↑** — jump
- **F / J** — melee (bosses: only during **stun/open**)
- **E** at the stall — open shop; **1/2/3** buy; **E/Esc** close
- Touch the **lantern** to save a rest spot
- Path: `meadow` → `ruins` → `grove` → `crystal` → `atelier` (final) → meadow

## Layout

- `assets/levels` — `meadow_01`, `ruins_01`, `grove_01`, `crystal_01`, `atelier_01`
- `assets/shop` — item catalog
- `src/app` — window + fixed-timestep loop + pixel scale
- `src/sim` — level, combat, shop, bosses, platforms, projectiles
- `src/render` — draw current sim state
- `docs` — design bible


