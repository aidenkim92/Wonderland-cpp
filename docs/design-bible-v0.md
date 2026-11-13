# Wonderland — Design Bible v0

Bright pixel adventure remake. Spec for agents and humans. Not a Unity port.

## Pitch

A developer dozes off at the keyboard and wakes inside their own game — a sunlit world of grass, stone, and cheerful danger. Learn to move, fight, and explore. Escape by finishing what you built.

Tone: warm, readable, lightly funny. Never grimdark.

## Pillars

1. **Readable first** — silhouette and color tell friend from foe in one glance.
2. **Teach by rooms** — each space adds one idea; bosses test the last few ideas.
3. **Short loops** — explore → small fight → platform/puzzle beat → checkpoint rest.
4. **Data-driven world** — levels are content files; code is a fixed-timestep sim.

## Visual

| Rule | Choice |
|---|---|
| Tile size | 16×16 px |
| Actor size | 32×32 px (drawn on 16 grid) |
| Camera | Integer / pixel-perfect scale (start at 3×) |
| Sky | `#7EC8E3` |
| Grass | `#5DBB63` → `#3D8B4F` |
| Stone | `#8FA3B0` → `#5C6B75` |
| Accent (coins, UI focus) | `#F4C15D` |
| Danger / hit flash | `#E85A4F` |
| Player tunic | Coral `#E07A5F` on cream skin, teal scarf |

UI: chunky pixel panels, large hearts and coin digits. One job per screen (title / pause / shop). No dense dashboards.

## Audio

- BGM: mid-tempo chip + soft acoustic bed; field bright, dungeon slightly more rhythmic, boss punchier but still major-key leaning.
- SFX: short high blips for jump/coin/UI; sword = crisp slice; hurt = soft thud (not horror sting).
- Place by trigger name in code (`sfx.jump`, `sfx.coin`, …). Source assets curated to this guide.

## Player feel (slice 0 targets)

| Param | Value | Notes |
|---|---|---|
| Max run speed | 120 px/s | World units = pixels at 1× |
| Gravity | 900 px/s² | Fixed dt 1/60 |
| Jump velocity | −320 px/s | Single jump first; double jump later |
| Coyote time | 80 ms | Leave ground, still jumpable |
| Jump buffer | 100 ms | Press slightly early still jumps |
| Ground | AABB platforms | Box2D later if needed |

Controls (keyboard): A/D or ←/→ move, Space/W/↑ jump. Gamepad later.

## Content roadmap (vertical)

1. **Slice 0** — window, fixed loop, player move/jump on a sunny stub meadow.
2. **Slice 1** — tilemap load from `assets/levels/*.json`, one patrol blob, melee hitbox (F/J), HP + touch damage.
3. **Slice 2** — lantern checkpoint (heal + respawn), door transitions, `ruins_01` room, spike tiles (`^`).
4. **Slice 3 (now)** — shop catalog data, coins, stall UI (heal / damage tonic / extra heart).
5. **Slice 4** — mid-boss state machine; then expand zones.

## Level intent (field → deep)

| Zone | Intent |
|---|---|
| Prologue meadow | Move, jump, fall hurt optional; no real enemies |
| Sunken ruins 1 | Spikes + one moving platform; bright interiors |
| Mid-boss grove | One new rule (e.g. shield bash / reflect) |
| Crystal deep | Combo geometry; palette stays saturated |
| Final atelier | Pattern-learn boss; bright stage lighting |

Rooms are authored as data (`assets/levels/*.json`). Tile `#` = solid, `^` = spike hazard, `.` = air. Optional fields: `theme`, `checkpoint`, `exit.to`. Checkpoints are lantern props — stepping on one lights it, heals, and becomes the respawn rest spot.

## Economy (slice 3)

Few clear power-ups, not deep stat sheets. Start with 2 coins; blobs drop coins. Catalog in `assets/shop/catalog.json`: Honey bun (heal), Coral tonic (damage, once), Extra heart (max HP, once). Progress persists across room transitions.

## Non-goals (v0)

- Unity scene parity or MonoBehaviour-shaped code
- Full custom renderer / Vulkan
- Large cutscenes before playable combat loop
- Porting old C# class graphs

## Sync

- Remake repo: `Wonderland-cpp`
- Legacy reference only: [aidenkim92/Wonderland](https://github.com/aidenkim92/Wonderland) (feel & feature list, not architecture)
