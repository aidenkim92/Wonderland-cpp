#include "render/Draw.hpp"

#include "sim/Sim.hpp"

#include <raylib.h>

namespace wonderland {
namespace {

constexpr Color kSkyMeadow{126, 200, 227, 255};
constexpr Color kSkyRuins{186, 214, 227, 255};
constexpr Color kGrassLight{93, 187, 99, 255};
constexpr Color kGrassDark{61, 139, 79, 255};
constexpr Color kStone{143, 163, 176, 255};
constexpr Color kStoneDark{92, 107, 117, 255};
constexpr Color kStoneWarm{168, 176, 186, 255};
constexpr Color kPlayer{224, 122, 95, 255};
constexpr Color kScarf{56, 142, 142, 255};
constexpr Color kAccent{244, 193, 93, 255};
constexpr Color kDanger{232, 90, 79, 255};
constexpr Color kBlob{120, 196, 140, 255};
constexpr Color kHeart{232, 90, 79, 255};
constexpr Color kDoor{120, 92, 70, 255};
constexpr Color kDoorLit{244, 193, 93, 255};

}  // namespace

void drawFrame(const Sim& sim) {
  const bool ruins = sim.level.theme == "ruins";
  ClearBackground(ruins ? kSkyRuins : kSkyMeadow);

  if (!ruins) {
    DrawEllipse(160, static_cast<int>(sim.worldH) - 80, 180, 50,
                Fade(kGrassDark, 0.3f));
    DrawEllipse(420, static_cast<int>(sim.worldH) - 100, 220, 60,
                Fade(kGrassDark, 0.25f));
  }

  const int t = sim.level.tile;
  for (int r = 0; r < sim.level.rows; ++r) {
    for (int c = 0; c < sim.level.cols; ++c) {
      const int x = c * t;
      const int y = r * t;
      if (sim.level.spike(c, r)) {
        DrawTriangle(Vector2{static_cast<float>(x), static_cast<float>(y + t)},
                     Vector2{static_cast<float>(x + t), static_cast<float>(y + t)},
                     Vector2{static_cast<float>(x + t * 0.5f),
                             static_cast<float>(y + 2)},
                     kDanger);
        continue;
      }
      if (!sim.level.solid(c, r)) {
        continue;
      }
      const bool topExposed = !sim.level.solid(c, r - 1);
      if (ruins) {
        DrawRectangle(x, y, t, t, topExposed ? kStoneWarm : kStone);
        DrawRectangle(x, y + t - 2, t, 2, kStoneDark);
      } else {
        DrawRectangle(x, y, t, t, topExposed ? kGrassLight : kStone);
        if (topExposed) {
          DrawRectangle(x, y, t, 3, kGrassDark);
        } else {
          DrawRectangle(x, y + t - 2, t, 2, kStoneDark);
        }
      }
    }
  }

  if (sim.checkpoint.exists) {
    const auto& b = sim.checkpoint.box;
    const int lx = static_cast<int>(b.x + b.w * 0.5f);
    const int by = static_cast<int>(b.y + b.h);
    DrawRectangle(lx - 3, by - 28, 6, 28, kStoneDark);
    DrawCircle(lx, by - 34, sim.checkpoint.active ? 11 : 8,
               sim.checkpoint.active ? kAccent : Fade(kAccent, 0.45f));
  }

  if (sim.exitDoor.exists) {
    const auto& d = sim.exitDoor.box;
    DrawRectangle(static_cast<int>(d.x), static_cast<int>(d.y),
                  static_cast<int>(d.w), static_cast<int>(d.h), kDoor);
    DrawRectangle(static_cast<int>(d.x + 4), static_cast<int>(d.y + 6),
                  static_cast<int>(d.w - 8), static_cast<int>(d.h - 10),
                  Fade(kDoorLit, 0.55f));
  }

  if (sim.enemy.alive) {
    const auto& e = sim.enemy.box;
    const Color body = sim.enemy.hitFlash > 0 ? kDanger : kBlob;
    DrawRectangle(static_cast<int>(e.x), static_cast<int>(e.y),
                  static_cast<int>(e.w), static_cast<int>(e.h), body);
    DrawCircle(static_cast<int>(e.x + 8), static_cast<int>(e.y + 10), 3, BLACK);
    DrawCircle(static_cast<int>(e.x + 18), static_cast<int>(e.y + 10), 3, BLACK);
  }

  const bool blink = sim.player.invuln > 0.0f &&
                     (static_cast<int>(sim.player.invuln * 20) % 2 == 0);
  if (!blink) {
    const auto& p = sim.player.box;
    DrawRectangle(static_cast<int>(p.x), static_cast<int>(p.y),
                  static_cast<int>(p.w), static_cast<int>(p.h), kPlayer);
    DrawRectangle(static_cast<int>(p.x + 8), static_cast<int>(p.y + 4), 16, 8,
                  kScarf);
  }

  if (sim.playerAttacking()) {
    const AABB hit = sim.attackBox();
    DrawRectangle(static_cast<int>(hit.x), static_cast<int>(hit.y),
                  static_cast<int>(hit.w), static_cast<int>(hit.h),
                  Fade(kAccent, 0.75f));
  }

  for (int i = 0; i < sim.player.maxHp; ++i) {
    const int hx = 16 + i * 22;
    DrawRectangle(hx, 14, 16, 14,
                  i < sim.player.hp ? kHeart : Fade(kHeart, 0.25f));
  }

  DrawText("Wonderland — slice 2", 16, 36, 18, Fade(BLACK, 0.55f));
  DrawText("Lantern saves rest · Door changes room · Spikes hurt", 16, 56, 15,
           Fade(BLACK, 0.45f));
  if (!sim.status.empty()) {
    DrawText(sim.status.c_str(), 16, 76, 16, Fade(BLACK, 0.5f));
  }
}

}  // namespace wonderland
