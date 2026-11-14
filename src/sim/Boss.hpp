#pragma once

#include "sim/Geom.hpp"
#include "sim/Level.hpp"

#include <string>

namespace wonderland {

struct Player;

enum class BossState {
  Idle,
  WindUp,
  Charge,
  Recover,
  Stunned,
  Dead,
};

struct Boss {
  bool present = false;
  bool alive = false;
  BossState state = BossState::Idle;
  AABB box{0, 0, 48, 48};
  float vx = 0;
  float vy = 0;
  int facing = -1;
  int hp = 8;
  int maxHp = 8;
  int coinDrop = 10;
  float timer = 0;
  float hitFlash = 0;
  float arenaMinX = 0;
  float arenaMaxX = 0;
  std::string type;

  void spawn(const Level& level, int col, int row, float worldW);
  void fixedUpdate(float dt, const Level& level, const Player& player);
  bool tryMeleeHit(int damage, int hitFacing);
  bool invulnerable() const;
  const char* stateName() const;
};

}  // namespace wonderland
