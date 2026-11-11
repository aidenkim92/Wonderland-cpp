#include "render/Draw.hpp"

#include "sim/Sim.hpp"

#include <raylib.h>

#include <string>

namespace wonderland {
namespace {

constexpr Color kSky{126, 200, 227, 255};
constexpr Color kGrassLight{93, 187, 99, 255};
constexpr Color kGrassDark{61, 139, 79, 255};
constexpr Color kStone{143, 163, 176, 255};
constexpr Color kStoneDark{92, 107, 117, 255};
constexpr Color kPlayer{224, 122, 95, 255};
constexpr Color kScarf{56, 142, 142, 255};
constexpr Color kAccent{244, 193, 93, 255};
constexpr Color kDanger{232, 90, 79, 255};
constexpr Color kBlob{120, 196, 140, 255};
constexpr Color kHeart{232, 90, 79, 255};

}  // namespace

void drawFrame(const Sim& sim) {
  ClearBackground(kSky);

  DrawEllipse(160, static_cast<int>(sim.worldH) - 80, 180, 50,
              Fade(kGrassDark, 0.3f));
  DrawEllipse(420, static_cast<int>(sim.worldH) - 100, 220, 60,
              Fade(kGrassDark, 0.25f));

  const int t = sim.level.tile;
  for (int r = 0; r < sim.level.rows; ++r) {
    for (int c = 0; c < sim.level.cols; ++c) {
      if (!sim.level.solid(c, r)) {
        continue;
      }
      const int x = c * t;
      const int y = r * t;
      const bool grassTop = !sim.level.solid(c, r - 1);
      DrawRectangle(x, y, t, t, grassTop ? kGrassLight : kStone);
      if (grassTop) {
        DrawRectangle(x, y, t, 3, kGrassDark);
      } else {
        DrawRectangle(x, y + t - 2, t, 2, kStoneDark);
      }
    }
  }

  // Lantern prop near right side of ground
  const int lanternX = static_cast<int>(sim.worldW) - 80;
  const int groundY = (sim.level.rows - 3) * t;
  DrawRectangle(lanternX, groundY - 28, 8, 28, kStoneDark);
  DrawCircle(lanternX + 4, groundY - 34, 9, kAccent);

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
    DrawRectangle(hx, 14, 16, 14, i < sim.player.hp ? kHeart : Fade(kHeart, 0.25f));
  }

  DrawText("Wonderland — slice 1", 16, 36, 18, Fade(BLACK, 0.55f));
  DrawText("A/D move · Space jump · F/J attack", 16, 56, 16, Fade(BLACK, 0.45f));
  if (!sim.status.empty()) {
    DrawText(sim.status.c_str(), 16, 76, 16, Fade(BLACK, 0.5f));
  }
}

}  // namespace wonderland
