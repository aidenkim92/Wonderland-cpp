#include "render/Draw.hpp"

#include "sim/Sim.hpp"

#include <raylib.h>

namespace wonderland {
namespace {

// Design bible v0 palette
constexpr Color kSky{126, 200, 227, 255};
constexpr Color kGrassLight{93, 187, 99, 255};
constexpr Color kGrassDark{61, 139, 79, 255};
constexpr Color kStone{143, 163, 176, 255};
constexpr Color kPlayer{224, 122, 95, 255};
constexpr Color kScarf{56, 142, 142, 255};
constexpr Color kAccent{244, 193, 93, 255};

}  // namespace

void drawFrame(const Sim& sim) {
  ClearBackground(kSky);

  // Far hills (cheap parallax feel)
  DrawEllipse(180, 360, 220, 70, Fade(kGrassDark, 0.35f));
  DrawEllipse(520, 340, 280, 90, Fade(kGrassDark, 0.3f));
  DrawEllipse(820, 355, 200, 65, Fade(kGrassDark, 0.35f));

  const auto& g = sim.ground;
  DrawRectangle(static_cast<int>(g.x), static_cast<int>(g.y),
                static_cast<int>(g.w), static_cast<int>(g.h), kGrassLight);
  DrawRectangle(static_cast<int>(g.x), static_cast<int>(g.y),
                static_cast<int>(g.w), 10, kGrassDark);
  DrawRectangle(static_cast<int>(g.x), static_cast<int>(g.y + 48),
                static_cast<int>(g.w), static_cast<int>(g.h - 48), kStone);

  // Rest lantern hint (checkpoint prop later)
  DrawRectangle(700, static_cast<int>(g.y) - 28, 10, 28, kStone);
  DrawCircle(705, static_cast<int>(g.y) - 34, 10, kAccent);

  const auto& p = sim.player.box;
  DrawRectangle(static_cast<int>(p.x), static_cast<int>(p.y),
                static_cast<int>(p.w), static_cast<int>(p.h), kPlayer);
  DrawRectangle(static_cast<int>(p.x + 8), static_cast<int>(p.y + 4), 16, 8,
                kScarf);

  DrawText("Wonderland — slice 0", 16, 12, 20, Fade(BLACK, 0.55f));
  DrawText("A/D move · Space jump", 16, 36, 16, Fade(BLACK, 0.45f));
}

}  // namespace wonderland
