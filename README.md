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

## Controls (slice 2)

- **A / D** or arrows — move
- **Space / W / ↑** — jump
- **F / J** — melee attack
- Touch the **lantern** to save a rest spot (heal + respawn there)
- Walk into the **door** to change rooms (`meadow_01` ↔ `ruins_01`)

## Layout

- `assets/levels` — JSON tilemaps (`meadow_01`, `ruins_01`)
- `src/app` — window + fixed-timestep loop + pixel scale
- `src/sim` — level load, player, enemy, combat, checkpoints, exits
- `src/render` — draw current sim state
- `docs` — design bible
