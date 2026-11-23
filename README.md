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

## Controls (slice 7)

- Title: **1 / Enter** New Game · **2 / C** Continue (if save exists)
- **A / D** move · **Space** jump · **F / J** attack · **E** shop
- **F5** save · **F9** load
- Lantern light + room change **autosave** to `~/.wonderland_save.json`
- Path: `meadow` → `ruins` → `grove` → `crystal` → `atelier` → meadow

## Layout

- `assets/levels` — zone tilemaps
- `assets/shop` — item catalog
- `src/app` — title + game loop
- `src/sim` — sim, bosses, shop, **save/load**
- `src/render` — draw
- `docs` — design bible



