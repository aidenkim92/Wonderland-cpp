#include "app/Game.hpp"

#include "render/Draw.hpp"
#include "sim/Sim.hpp"

#include <raylib.h>

namespace wonderland {
namespace {

constexpr int kWindowW = 960;
constexpr int kWindowH = 540;
constexpr float kFixedDt = 1.0f / 60.0f;
constexpr float kMaxFrame = 0.25f;

}  // namespace

int Game::run() {
  SetConfigFlags(FLAG_WINDOW_HIGHDPI);
  InitWindow(kWindowW, kWindowH, "Wonderland");
  SetTargetFPS(60);

  Sim sim;
  sim.resetMeadow();

  float accumulator = 0.0f;

  while (!WindowShouldClose()) {
    float frame = GetFrameTime();
    if (frame > kMaxFrame) {
      frame = kMaxFrame;
    }
    accumulator += frame;

    const Input input = pollInput();
    while (accumulator >= kFixedDt) {
      sim.fixedUpdate(kFixedDt, input);
      accumulator -= kFixedDt;
    }

    BeginDrawing();
    drawFrame(sim);
    EndDrawing();
  }

  CloseWindow();
  return 0;
}

}  // namespace wonderland
