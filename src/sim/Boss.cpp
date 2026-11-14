#include "sim/Boss.hpp"

#include "sim/Sim.hpp"

#include <algorithm>
#include <cmath>

namespace wonderland {
namespace {

constexpr float kGravity = 900.0f;
constexpr float kIdleTime = 1.1f;
constexpr float kWindUpTime = 0.55f;
constexpr float kChargeTime = 0.75f;
constexpr float kChargeSpeed = 220.0f;
constexpr float kRecoverTime = 0.25f;
constexpr float kStunTime = 1.6f;

bool solidAtPx(const Level& level, float px, float py) {
  const int col =
      static_cast<int>(std::floor(px / static_cast<float>(level.tile)));
  const int row =
      static_cast<int>(std::floor(py / static_cast<float>(level.tile)));
  return level.solid(col, row);
}

void placeOnGround(const Level& level, AABB& box) {
  const float t = static_cast<float>(level.tile);
  for (int i = 0; i < level.rows * level.tile + 8; ++i) {
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

void moveAndCollide(const Level& level, AABB& box, float& vx, float& vy,
                    float dt) {
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

void enter(Boss& boss, BossState next, float duration) {
  boss.state = next;
  boss.timer = duration;
  boss.vx = 0;
}

}  // namespace

void Boss::spawn(const Level& level, int col, int row, float worldW) {
  (void)row;
  present = true;
  alive = true;
  type = "grove_brute";
  hp = maxHp;
  const float tile = static_cast<float>(level.tile);
  box = AABB{col * tile, 0.0f, 48.0f, 48.0f};
  placeOnGround(level, box);
  facing = -1;
  arenaMinX = tile * 2.0f;
  arenaMaxX = worldW - tile * 2.0f;
  hitFlash = 0;
  enter(*this, BossState::Idle, kIdleTime);
}

bool Boss::invulnerable() const {
  return state != BossState::Stunned && state != BossState::Dead;
}

const char* Boss::stateName() const {
  switch (state) {
    case BossState::Idle:
      return "idle";
    case BossState::WindUp:
      return "wind-up";
    case BossState::Charge:
      return "charge";
    case BossState::Recover:
      return "recover";
    case BossState::Stunned:
      return "stunned";
    case BossState::Dead:
      return "down";
  }
  return "?";
}

bool Boss::tryMeleeHit(int damage, int hitFacing) {
  (void)hitFacing;
  if (!alive || state == BossState::Dead) {
    return false;
  }
  if (invulnerable()) {
    hitFlash = 0.08f;
    return false;
  }
  hp -= damage;
  hitFlash = 0.2f;
  if (hp <= 0) {
    hp = 0;
    alive = false;
    enter(*this, BossState::Dead, 0.0f);
  }
  return true;
}

void Boss::fixedUpdate(float dt, const Level& level, const Player& player) {
  if (!present || !alive) {
    vx = 0;
    return;
  }

  hitFlash = std::max(0.0f, hitFlash - dt);
  timer = std::max(0.0f, timer - dt);

  const float playerCx = player.box.x + player.box.w * 0.5f;
  const float bossCx = box.x + box.w * 0.5f;

  switch (state) {
    case BossState::Idle:
      facing = playerCx < bossCx ? -1 : 1;
      vx = 0;
      if (timer <= 0.0f) {
        enter(*this, BossState::WindUp, kWindUpTime);
      }
      break;

    case BossState::WindUp:
      facing = playerCx < bossCx ? -1 : 1;
      vx = 0;
      if (timer <= 0.0f) {
        enter(*this, BossState::Charge, kChargeTime);
        vx = static_cast<float>(facing) * kChargeSpeed;
      }
      break;

    case BossState::Charge: {
      float fall = 0.0f;
      moveAndCollide(level, box, vx, fall, dt);
      if (box.x < arenaMinX) {
        box.x = arenaMinX;
        vx = 0;
        timer = 0;
      }
      if (box.x + box.w > arenaMaxX) {
        box.x = arenaMaxX - box.w;
        vx = 0;
        timer = 0;
      }
      if (timer <= 0.0f || vx == 0.0f) {
        enter(*this, BossState::Recover, kRecoverTime);
      }
      break;
    }

    case BossState::Recover:
      vx = 0;
      if (timer <= 0.0f) {
        enter(*this, BossState::Stunned, kStunTime);
      }
      break;

    case BossState::Stunned:
      vx = 0;
      if (timer <= 0.0f) {
        enter(*this, BossState::Idle, kIdleTime);
      }
      break;

    case BossState::Dead:
      vx = 0;
      break;
  }

  // Keep feet planted.
  float zero = 0.0f;
  float fall = kGravity * dt;
  moveAndCollide(level, box, zero, fall, dt);
}

}  // namespace wonderland
