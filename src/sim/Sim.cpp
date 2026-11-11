#include "sim/Sim.hpp"

#include <raylib.h>

#include <algorithm>
#include <cmath>

namespace wonderland {
namespace {

// Design bible v0 — player feel
constexpr float kMaxRun = 120.0f;
constexpr float kAccel = 900.0f;
constexpr float kFriction = 800.0f;
constexpr float kGravity = 900.0f;
constexpr float kJumpV = -320.0f;
constexpr float kCoyote = 0.08f;
constexpr float kJumpBuf = 0.10f;
constexpr float kJumpCut = 0.45f;

bool overlap(const AABB& a, const AABB& b) {
  return a.x < b.x + b.w && a.x + a.w > b.x && a.y < b.y + b.h && a.y + a.h > b.y;
}

}  // namespace

Input pollInput() {
  Input in;
  in.left = IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT);
  in.right = IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT);
  in.jumpPressed =
      IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_W) || IsKeyPressed(KEY_UP);
  in.jumpHeld = IsKeyDown(KEY_SPACE) || IsKeyDown(KEY_W) || IsKeyDown(KEY_UP);
  return in;
}

void Sim::resetMeadow() {
  worldW = 960;
  worldH = 540;
  ground = AABB{0, 420, worldW, 120};
  player.box = AABB{120, ground.y - 32, 32, 32};
  player.vx = 0;
  player.vy = 0;
  player.grounded = true;
  player.coyote = kCoyote;
  player.jumpBuffer = 0;
}

void Sim::fixedUpdate(float dt, const Input& input) {
  if (input.jumpPressed) {
    player.jumpBuffer = kJumpBuf;
  } else {
    player.jumpBuffer = std::max(0.0f, player.jumpBuffer - dt);
  }

  float wish = 0.0f;
  if (input.left) {
    wish -= 1.0f;
  }
  if (input.right) {
    wish += 1.0f;
  }

  if (wish != 0.0f) {
    player.vx += wish * kAccel * dt;
    player.vx = std::clamp(player.vx, -kMaxRun, kMaxRun);
  } else if (player.grounded) {
    const float decel = kFriction * dt;
    if (std::fabs(player.vx) <= decel) {
      player.vx = 0;
    } else {
      player.vx -= std::copysign(decel, player.vx);
    }
  }

  if (player.grounded) {
    player.coyote = kCoyote;
  } else {
    player.coyote = std::max(0.0f, player.coyote - dt);
  }

  const bool canJump = player.grounded || player.coyote > 0.0f;
  if (player.jumpBuffer > 0.0f && canJump) {
    player.vy = kJumpV;
    player.grounded = false;
    player.coyote = 0;
    player.jumpBuffer = 0;
  }

  if (!input.jumpHeld && player.vy < 0.0f) {
    player.vy *= kJumpCut;
  }

  player.vy += kGravity * dt;

  player.box.x += player.vx * dt;
  player.box.y += player.vy * dt;

  player.grounded = false;
  if (overlap(player.box, ground) && player.vy >= 0.0f) {
    player.box.y = ground.y - player.box.h;
    player.vy = 0;
    player.grounded = true;
  }

  if (player.box.x < 0) {
    player.box.x = 0;
    player.vx = 0;
  }
  if (player.box.x + player.box.w > worldW) {
    player.box.x = worldW - player.box.w;
    player.vx = 0;
  }

  if (player.box.y > worldH + 64) {
    resetMeadow();
  }
}

}  // namespace wonderland
