#include "app/Game.hpp"

#include "render/Draw.hpp"
#include "sim/Save.hpp"
#include "sim/Sim.hpp"

#include <raylib.h>

#include <string>

#ifndef WL_ASSETS_DIR
#define WL_ASSETS_DIR "assets"
#endif

namespace wonderland {
namespace {

constexpr float kFixedDt = 1.0f / 60.0f;
constexpr float kMaxFrame = 0.25f;
constexpr int kScale = 3;

enum class AppMode { Title, Play };

}  // namespace

int Game::run() {
  SetConfigFlags(FLAG_WINDOW_HIGHDPI);

  Sim sim;
  sim.assetsDir = WL_ASSETS_DIR;
  sim.savePath = defaultSavePath();

  std::string error;
  if (!sim.loadCatalog(error)) {
    TraceLog(LOG_ERROR, "Failed to load shop catalog: %s", error.c_str());
    return 1;
  }
  if (!sim.loadNamedLevel("meadow_01.json", error)) {
    TraceLog(LOG_ERROR, "Failed to load level: %s", error.c_str());
    return 1;
  }

  const int windowW = static_cast<int>(sim.worldW) * kScale;
  const int windowH = static_cast<int>(sim.worldH) * kScale;
  InitWindow(windowW, windowH, "Wonderland");
  SetTargetFPS(60);

  RenderTexture2D target =
      LoadRenderTexture(static_cast<int>(sim.worldW), static_cast<int>(sim.worldH));
  SetTextureFilter(target.texture, TEXTURE_FILTER_POINT);

  AppMode mode = AppMode::Title;
  const bool canContinue = saveExists(sim.savePath);
  float accumulator = 0.0f;

  while (!WindowShouldClose()) {
    float frame = GetFrameTime();
    if (frame > kMaxFrame) {
      frame = kMaxFrame;
    }
    accumulator += frame;

    if (mode == AppMode::Title) {
      if (IsKeyPressed(KEY_ONE) || IsKeyPressed(KEY_N) ||
          IsKeyPressed(KEY_ENTER)) {
        // Fresh run from meadow defaults.
        sim.player = Player{};
        if (!sim.loadNamedLevel("meadow_01.json", error)) {
          TraceLog(LOG_ERROR, "New game failed: %s", error.c_str());
        } else {
          sim.status = "new adventure";
          mode = AppMode::Play;
          accumulator = 0.0f;
        }
      } else if (canContinue &&
                 (IsKeyPressed(KEY_TWO) || IsKeyPressed(KEY_C))) {
        if (!sim.loadProgress(error)) {
          TraceLog(LOG_ERROR, "Continue failed: %s", error.c_str());
          sim.status = "continue failed";
        } else {
          mode = AppMode::Play;
          accumulator = 0.0f;
          if (static_cast<int>(sim.worldW) != target.texture.width ||
              static_cast<int>(sim.worldH) != target.texture.height) {
            UnloadRenderTexture(target);
            target = LoadRenderTexture(static_cast<int>(sim.worldW),
                                       static_cast<int>(sim.worldH));
            SetTextureFilter(target.texture, TEXTURE_FILTER_POINT);
          }
        }
      }

      BeginTextureMode(target);
      drawTitle(canContinue);
      EndTextureMode();
    } else {
      const Input input = pollInput();
      while (accumulator >= kFixedDt) {
        sim.fixedUpdate(kFixedDt, input);
        accumulator -= kFixedDt;
      }

      if (sim.saveRequested) {
        std::string saveError;
        if (!sim.saveProgress(saveError)) {
          TraceLog(LOG_WARNING, "Autosave failed: %s", saveError.c_str());
          sim.status = "save failed";
          sim.saveRequested = false;
        }
      }

      if (!sim.pendingLevel.empty()) {
        const std::string next = sim.pendingLevel;
        if (!sim.loadNamedLevel(next, error)) {
          TraceLog(LOG_ERROR, "Failed to load %s: %s", next.c_str(),
                   error.c_str());
          sim.pendingLevel.clear();
          sim.status = "door jammed";
        } else {
          if (static_cast<int>(sim.worldW) != target.texture.width ||
              static_cast<int>(sim.worldH) != target.texture.height) {
            UnloadRenderTexture(target);
            target = LoadRenderTexture(static_cast<int>(sim.worldW),
                                       static_cast<int>(sim.worldH));
            SetTextureFilter(target.texture, TEXTURE_FILTER_POINT);
          }
          std::string saveError;
          sim.saveProgress(saveError);
        }
      }

      BeginTextureMode(target);
      drawFrame(sim);
      EndTextureMode();
    }

    BeginDrawing();
    ClearBackground(BLACK);
    const Rectangle src{0, 0, static_cast<float>(target.texture.width),
                        -static_cast<float>(target.texture.height)};
    const Rectangle dst{0, 0, static_cast<float>(windowW),
                        static_cast<float>(windowH)};
    DrawTexturePro(target.texture, src, dst, Vector2{0, 0}, 0.0f, WHITE);
    EndDrawing();
  }

  UnloadRenderTexture(target);
  CloseWindow();
  return 0;
}

}  // namespace wonderland
