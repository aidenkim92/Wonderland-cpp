#include "sim/Sim.hpp"

#include <raylib.h>

#include <algorithm>
#include <cmath>

namespace wonderland {
namespace {

constexpr float kMaxRun = 120.0f;
constexpr float kAccel = 900.0f;
constexpr float kFriction = 800.0f;
constexpr float kGravity = 900.0f;
constexpr float kJumpV = -320.0f;
constexpr float kCoyote = 0.08f;
constexpr float kJumpBuf = 0.10f;
constexpr float kJumpCut = 0.45f;
constexpr float kAttackCd = 0.35f;
constexpr float kAttackActive = 0.12f;
constexpr float kInvuln = 0.8f;
constexpr float kKnock = 160.0f;

bool overlap(const AABB& a, const AABB& b) {
  return a.x < b.x + b.w && a.x + a.w > b.x && a.y < b.y + b.h &&
         a.y + a.h > b.y;
}

bool solidAtPx(const Level& level, float px, float py) {
  const int col = static_cast<int>(std::floor(px / static_cast<float>(level.tile)));
  const int row = static_cast<int>(std::floor(py / static_cast<float>(level.tile)));
  return level.solid(col, row);
}

void moveAndCollide(const Level& level, AABB& box, float& vx, float& vy,
                    float dt, bool& grounded) {
  grounded = false;
  const float t = static_cast<float>(level.tile);

  box.x += vx * dt;
  if (vx > 0.0f) {
    if (solidAtPx(level, box.x + box.w - 0.001f, box.y + 1.0f) ||
        solidAtPx(level, box.x + box.w - 0.001f, box.y + box.h - 1.0f)) {
      const int col =
          static_cast<int>(std::floor((box.x + box.w - 0.001f) / t));
      box.x = static_cast<float>(col) * t - box.w;
      vx = 0.0f;
    }
  } else if (vx < 0.0f) {
    if (solidAtPx(level, box.x, box.y + 1.0f) ||
        solidAtPx(level, box.x, box.y + box.h - 1.0f)) {
      const int col = static_cast<int>(std::floor(box.x / t));
      box.x = static_cast<float>(col + 1) * t;
      vx = 0.0f;
    }
  }

  box.y += vy * dt;
  if (vy > 0.0f) {
    if (solidAtPx(level, box.x + 1.0f, box.y + box.h - 0.001f) ||
        solidAtPx(level, box.x + box.w - 2.0f, box.y + box.h - 0.001f)) {
      const int row =
          static_cast<int>(std::floor((box.y + box.h - 0.001f) / t));
      box.y = static_cast<float>(row) * t - box.h;
      vy = 0.0f;
      grounded = true;
    }
  } else if (vy < 0.0f) {
    if (solidAtPx(level, box.x + 1.0f, box.y) ||
        solidAtPx(level, box.x + box.w - 2.0f, box.y)) {
      const int row = static_cast<int>(std::floor(box.y / t));
      box.y = static_cast<float>(row + 1) * t;
      vy = 0.0f;
    }
  }
}

void placeOnGround(const Level& level, AABB& box) {
  const float t = static_cast<float>(level.tile);
  // Drop from spawn until feet hit a solid.
  for (int i = 0; i < level.rows * 2; ++i) {
    const float feet = box.y + box.h;
    if (solidAtPx(level, box.x + 2.0f, feet) ||
        solidAtPx(level, box.x + box.w - 3.0f, feet)) {
      const int row = static_cast<int>(std::floor(feet / t));
      box.y = static_cast<float>(row) * t - box.h;
      return;
    }
    box.y += 1.0f;
  }
}

}  // namespace

Input pollInput() {
  Input in;
  in.left = IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT);
  in.right = IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT);
  in.jumpPressed =
      IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_W) || IsKeyPressed(KEY_UP);
  in.jumpHeld = IsKeyDown(KEY_SPACE) || IsKeyDown(KEY_W) || IsKeyDown(KEY_UP);
  in.attackPressed = IsKeyPressed(KEY_F) || IsKeyPressed(KEY_J);
  return in;
}

AABB Sim::attackBox() const {
  constexpr float w = 22.0f;
  constexpr float h = 20.0f;
  const float x = player.facing > 0 ? player.box.x + player.box.w - 4.0f
                                    : player.box.x - w + 4.0f;
  return AABB{x, player.box.y + 8.0f, w, h};
}

bool Sim::loadFromFile(const std::string& path, std::string& error) {
  Level loaded;
  if (!loadLevelFile(path, loaded, error)) {
    return false;
  }
  level = std::move(loaded);
  worldW = static_cast<float>(level.cols * level.tile);
  worldH = static_cast<float>(level.rows * level.tile);

  const float tile = static_cast<float>(level.tile);
  player.box = AABB{level.spawnCol * tile + 2.0f, 0.0f, 32.0f, 32.0f};
  placeOnGround(level, player.box);
  player.vx = 0;
  player.vy = 0;
  player.grounded = true;
  player.coyote = kCoyote;
  player.jumpBuffer = 0;
  player.facing = 1;
  player.hp = player.maxHp;
  player.invuln = 0;
  player.attackCd = 0;
  player.attackActive = 0;

  const float ex = level.enemy.col * tile;
  const float half = static_cast<float>(std::max(1, level.enemy.patrol)) * tile;
  enemy.alive = true;
  enemy.box = AABB{ex, 0.0f, 28.0f, 28.0f};
  placeOnGround(level, enemy.box);
  enemy.vx = 40.0f;
  enemy.minX = std::max(0.0f, ex - half);
  enemy.maxX = std::min(worldW, ex + half + enemy.box.w);
  enemy.hp = 2;
  enemy.hitFlash = 0;

  status = level.name;
  return true;
}

void Sim::respawnPlayer() {
  const float tile = static_cast<float>(level.tile);
  player.box = AABB{level.spawnCol * tile + 2.0f, 0.0f, 32.0f, 32.0f};
  placeOnGround(level, player.box);
  player.vx = 0;
  player.vy = 0;
  player.grounded = true;
  player.coyote = kCoyote;
  player.jumpBuffer = 0;
  player.hp = player.maxHp;
  player.invuln = 1.0f;
  player.attackCd = 0;
  player.attackActive = 0;
}

void Sim::fixedUpdate(float dt, const Input& input) {
  player.invuln = std::max(0.0f, player.invuln - dt);
  player.attackCd = std::max(0.0f, player.attackCd - dt);
  player.attackActive = std::max(0.0f, player.attackActive - dt);
  enemy.hitFlash = std::max(0.0f, enemy.hitFlash - dt);

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
    player.facing = wish > 0 ? 1 : -1;
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

  if (player.jumpBuffer > 0.0f && (player.grounded || player.coyote > 0.0f)) {
    player.vy = kJumpV;
    player.grounded = false;
    player.coyote = 0;
    player.jumpBuffer = 0;
  }

  if (!input.jumpHeld && player.vy < 0.0f) {
    player.vy *= kJumpCut;
  }

  if (input.attackPressed && player.attackCd <= 0.0f) {
    player.attackCd = kAttackCd;
    player.attackActive = kAttackActive;
  }

  player.vy += kGravity * dt;
  moveAndCollide(level, player.box, player.vx, player.vy, dt, player.grounded);

  if (enemy.alive) {
    float zero = 0.0f;
    bool ignored = false;
    moveAndCollide(level, enemy.box, enemy.vx, zero, dt, ignored);
    if (enemy.box.x <= enemy.minX) {
      enemy.box.x = enemy.minX;
      enemy.vx = std::fabs(enemy.vx);
    } else if (enemy.box.x + enemy.box.w >= enemy.maxX) {
      enemy.box.x = enemy.maxX - enemy.box.w;
      enemy.vx = -std::fabs(enemy.vx);
    }

    float fall = 220.0f;
    float noX = 0.0f;
    bool groundedEnemy = false;
    moveAndCollide(level, enemy.box, noX, fall, dt, groundedEnemy);
    if (!groundedEnemy) {
      enemy.vx = -enemy.vx;
    }
  }

  if (playerAttacking() && enemy.alive && overlap(attackBox(), enemy.box)) {
    enemy.hp -= 1;
    enemy.hitFlash = 0.15f;
    enemy.box.x += static_cast<float>(player.facing) * 14.0f;
    player.attackActive = 0;
    if (enemy.hp <= 0) {
      enemy.alive = false;
      status = "blob down!";
    }
  }

  if (enemy.alive && player.invuln <= 0.0f && overlap(player.box, enemy.box)) {
    player.hp -= 1;
    player.invuln = kInvuln;
    player.vx =
        (player.box.x + player.box.w * 0.5f < enemy.box.x + enemy.box.w * 0.5f)
            ? -kKnock
            : kKnock;
    player.vy = -120.0f;
    if (player.hp <= 0) {
      respawnPlayer();
      status = "respawned";
    }
  }

  if (player.box.y > worldH + 64.0f) {
    respawnPlayer();
    status = "fell — respawn";
  }
}

}  // namespace wonderland
