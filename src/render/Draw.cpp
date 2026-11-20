#include "render/Draw.hpp"

#include "sim/Sim.hpp"

#include <raylib.h>

#include <string>

namespace wonderland {
namespace {

constexpr Color kSkyMeadow{126, 200, 227, 255};
constexpr Color kSkyRuins{186, 214, 227, 255};
constexpr Color kSkyGrove{168, 214, 180, 255};
constexpr Color kSkyCrystal{190, 230, 240, 255};
constexpr Color kSkyAtelier{255, 236, 210, 255};
constexpr Color kGrassLight{93, 187, 99, 255};
constexpr Color kGrassDark{61, 139, 79, 255};
constexpr Color kStone{143, 163, 176, 255};
constexpr Color kStoneDark{92, 107, 117, 255};
constexpr Color kStoneWarm{168, 176, 186, 255};
constexpr Color kCrystal{110, 198, 210, 255};
constexpr Color kCrystalHi{170, 230, 236, 255};
constexpr Color kPad{90, 170, 200, 255};
constexpr Color kPlayer{224, 122, 95, 255};
constexpr Color kScarf{56, 142, 142, 255};
constexpr Color kAccent{244, 193, 93, 255};
constexpr Color kDanger{232, 90, 79, 255};
constexpr Color kBlob{120, 196, 140, 255};
constexpr Color kHeart{232, 90, 79, 255};
constexpr Color kDoor{120, 92, 70, 255};
constexpr Color kDoorLit{244, 193, 93, 255};
constexpr Color kStall{196, 140, 92, 255};
constexpr Color kPanel{255, 248, 230, 255};
constexpr Color kPanelEdge{120, 92, 70, 255};
constexpr Color kBoss{214, 140, 72, 255};
constexpr Color kBossStun{244, 210, 120, 255};
constexpr Color kBossShield{120, 160, 200, 255};
constexpr Color kMuse{180, 120, 200, 255};
constexpr Color kMuseOpen{255, 200, 120, 255};
constexpr Color kInk{70, 60, 90, 255};
constexpr Color kSlam{255, 120, 80, 255};
constexpr Color kAtelierWood{210, 170, 120, 255};

Color skyFor(const std::string& theme) {
  if (theme == "ruins") {
    return kSkyRuins;
  }
  if (theme == "grove") {
    return kSkyGrove;
  }
  if (theme == "crystal") {
    return kSkyCrystal;
  }
  if (theme == "atelier") {
    return kSkyAtelier;
  }
  return kSkyMeadow;
}

}  // namespace

void drawFrame(const Sim& sim) {
  const std::string& theme = sim.level.theme;
  ClearBackground(skyFor(theme));

  if (theme != "ruins") {
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
      if (theme == "ruins") {
        DrawRectangle(x, y, t, t, topExposed ? kStoneWarm : kStone);
        DrawRectangle(x, y + t - 2, t, 2, kStoneDark);
      } else if (theme == "crystal") {
        DrawRectangle(x, y, t, t, topExposed ? kCrystalHi : kCrystal);
        DrawRectangle(x, y, t, 3, Fade(WHITE, 0.35f));
      } else if (theme == "atelier") {
        DrawRectangle(x, y, t, t, topExposed ? kAccent : kAtelierWood);
        if (topExposed) {
          DrawRectangle(x, y, t, 3, Fade(WHITE, 0.4f));
        }
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

  for (const MovingPlatform& plat : sim.platforms) {
    DrawRectangle(static_cast<int>(plat.box.x), static_cast<int>(plat.box.y),
                  static_cast<int>(plat.box.w), static_cast<int>(plat.box.h),
                  kPad);
    DrawRectangle(static_cast<int>(plat.box.x), static_cast<int>(plat.box.y),
                  static_cast<int>(plat.box.w), 3, kCrystalHi);
  }

  if (sim.shop.exists) {
    const auto& s = sim.shop.box;
    DrawRectangle(static_cast<int>(s.x), static_cast<int>(s.y + 8),
                  static_cast<int>(s.w), static_cast<int>(s.h - 8), kStall);
    DrawRectangle(static_cast<int>(s.x - 4), static_cast<int>(s.y),
                  static_cast<int>(s.w + 8), 10, kAccent);
    DrawCircle(static_cast<int>(s.x + s.w * 0.5f), static_cast<int>(s.y + 18),
               5, kPlayer);
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
                  sim.exitDoor.locked ? Fade(kDanger, 0.55f)
                                      : Fade(kDoorLit, 0.55f));
  }

  if (sim.enemy.alive) {
    const auto& e = sim.enemy.box;
    const Color body = sim.enemy.hitFlash > 0 ? kDanger : kBlob;
    DrawRectangle(static_cast<int>(e.x), static_cast<int>(e.y),
                  static_cast<int>(e.w), static_cast<int>(e.h), body);
    DrawCircle(static_cast<int>(e.x + 8), static_cast<int>(e.y + 10), 3, BLACK);
    DrawCircle(static_cast<int>(e.x + 18), static_cast<int>(e.y + 10), 3, BLACK);
  }

  if (sim.slam.active) {
    DrawRectangle(static_cast<int>(sim.slam.box.x),
                  static_cast<int>(sim.slam.box.y),
                  static_cast<int>(sim.slam.box.w),
                  static_cast<int>(sim.slam.box.h), Fade(kSlam, 0.75f));
  }

  for (const Projectile& shot : sim.projectiles) {
    if (!shot.alive) {
      continue;
    }
    DrawRectangle(static_cast<int>(shot.box.x), static_cast<int>(shot.box.y),
                  static_cast<int>(shot.box.w), static_cast<int>(shot.box.h),
                  kInk);
  }

  if (sim.boss.present && sim.boss.alive) {
    const auto& b = sim.boss.box;
    Color body = sim.boss.isMuse() ? kMuse : kBoss;
    if (sim.boss.state == BossState::Stunned) {
      body = sim.boss.isMuse() ? kMuseOpen : kBossStun;
    } else if (sim.boss.state == BossState::WindUp ||
               sim.boss.state == BossState::Charge ||
               sim.boss.state == BossState::SlamWindUp ||
               sim.boss.state == BossState::ShootWindUp ||
               sim.boss.state == BossState::HopWindUp) {
      body = kBossShield;
    }
    if (sim.boss.hitFlash > 0) {
      body = kDanger;
    }
    DrawRectangle(static_cast<int>(b.x), static_cast<int>(b.y),
                  static_cast<int>(b.w), static_cast<int>(b.h), body);
    DrawRectangle(static_cast<int>(b.x + 10), static_cast<int>(b.y + 12), 8, 8,
                  BLACK);
    DrawRectangle(static_cast<int>(b.x + b.w - 18), static_cast<int>(b.y + 12),
                  8, 8, BLACK);
    if (sim.boss.state == BossState::WindUp ||
        sim.boss.state == BossState::HopWindUp ||
        sim.boss.state == BossState::SlamWindUp) {
      const float tipX = sim.boss.facing > 0 ? b.x + b.w + 12 : b.x - 12;
      DrawCircle(static_cast<int>(tipX), static_cast<int>(b.y + 20), 6,
                 Fade(kDanger, 0.8f));
    }
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

  DrawRectangle(16, 34, 14, 14, kAccent);
  DrawText(TextFormat("x%d", sim.player.coins), 34, 34, 16, Fade(BLACK, 0.65f));
  DrawText(TextFormat("atk %d", sim.player.attackPower), 90, 34, 16,
           Fade(BLACK, 0.55f));

  if (sim.boss.present && sim.boss.alive) {
    const int barW = 160;
    const int barX = static_cast<int>(sim.worldW) - barW - 16;
    DrawRectangle(barX, 14, barW, 12, Fade(BLACK, 0.25f));
    const float pct =
        static_cast<float>(sim.boss.hp) / static_cast<float>(sim.boss.maxHp);
    DrawRectangle(barX, 14, static_cast<int>(barW * pct), 12,
                  sim.boss.isMuse() ? kMuse : kBoss);
    DrawText(TextFormat("%s [%s]%s", sim.boss.displayName(),
                        sim.boss.stateName(),
                        sim.boss.phase >= 2 ? " P2" : ""),
             barX, 30, 14, Fade(BLACK, 0.55f));
  }

  DrawText("Wonderland — slice 6", 16, 54, 18, Fade(BLACK, 0.55f));
  if (sim.nearShop && !sim.shopOpen) {
    DrawText("E — open stall", 16, 74, 15, Fade(BLACK, 0.55f));
  } else if (sim.boss.present && sim.boss.alive && sim.boss.isMuse()) {
    DrawText("Muse: hop/slam/ink — strike only when OPEN", 16, 74, 14,
             Fade(BLACK, 0.5f));
  } else if (sim.boss.present && sim.boss.alive) {
    DrawText("Boss: dodge charge, strike while gold/stunned", 16, 74, 14,
             Fade(BLACK, 0.5f));
  } else if (!sim.platforms.empty()) {
    DrawText("Ride cyan pads · watch spikes · door on the high ledge", 16, 74,
             14, Fade(BLACK, 0.5f));
  } else {
    DrawText("E shop · F/J attack · door travels", 16, 74, 15,
             Fade(BLACK, 0.45f));
  }
  if (!sim.status.empty()) {
    DrawText(sim.status.c_str(), 16, 94, 15, Fade(BLACK, 0.5f));
  }

  if (sim.shopOpen) {
    const int pw = 280;
    const int ph = 150;
    const int px = static_cast<int>(sim.worldW * 0.5f) - pw / 2;
    const int py = 70;
    DrawRectangle(px - 2, py - 2, pw + 4, ph + 4, kPanelEdge);
    DrawRectangle(px, py, pw, ph, kPanel);
    DrawText("Sunny Stall", px + 12, py + 10, 18, kPanelEdge);
    DrawText(TextFormat("Coins: %d", sim.player.coins), px + 160, py + 12, 14,
             Fade(BLACK, 0.65f));

    int line = 0;
    for (const ShopItem& item : sim.catalog.items) {
      if (line >= 3) {
        break;
      }
      const bool owned = sim.itemOwned(item);
      const std::string label =
          TextFormat("%d) %s  %dc%s", line + 1, item.name.c_str(), item.cost,
                     owned ? "  [owned]" : "");
      DrawText(label.c_str(), px + 12, py + 40 + line * 22, 15,
               owned ? Fade(BLACK, 0.35f) : Fade(BLACK, 0.75f));
      ++line;
    }
    DrawText("1/2/3 buy · E/Esc close", px + 12, py + ph - 24, 14,
             Fade(BLACK, 0.5f));
  }
}

}  // namespace wonderland
