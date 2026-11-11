#pragma once

#include "sim/Level.hpp"

#include <string>

namespace wonderland {

struct Input {
  bool left = false;
  bool right = false;
  bool jumpPressed = false;
  bool jumpHeld = false;
  bool attackPressed = false;
};

Input pollInput();

struct AABB {
  float x = 0;
  float y = 0;
  float w = 0;
  float h = 0;
};

struct Player {
  AABB box{0, 0, 32, 32};
  float vx = 0;
  float vy = 0;
  bool grounded = false;
  float coyote = 0;
  float jumpBuffer = 0;
  int facing = 1;  // +1 right, -1 left
  int hp = 3;
  int maxHp = 3;
  float invuln = 0;
  float attackCd = 0;
  float attackActive = 0;
};

struct Enemy {
  bool alive = false;
  AABB box{0, 0, 28, 28};
  float vx = 40;
  float minX = 0;
  float maxX = 0;
  int hp = 2;
  float hitFlash = 0;
};

struct Sim {
  Level level;
  Player player;
  Enemy enemy;
  float worldW = 640;
  float worldH = 288;
  std::string status;

  bool loadFromFile(const std::string& path, std::string& error);
  void respawnPlayer();
  void fixedUpdate(float dt, const Input& input);

  AABB attackBox() const;
  bool playerAttacking() const { return player.attackActive > 0; }
};

}  // namespace wonderland
