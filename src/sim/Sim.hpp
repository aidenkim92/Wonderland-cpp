#pragma once

#include "sim/Boss.hpp"
#include "sim/Geom.hpp"
#include "sim/Level.hpp"
#include "sim/Shop.hpp"

#include <string>

namespace wonderland {

struct Input {
  bool left = false;
  bool right = false;
  bool jumpPressed = false;
  bool jumpHeld = false;
  bool attackPressed = false;
  bool interactPressed = false;
  bool uiConfirm1 = false;
  bool uiConfirm2 = false;
  bool uiConfirm3 = false;
  bool uiClose = false;
};

Input pollInput();

struct Player {
  AABB box{0, 0, 32, 32};
  float vx = 0;
  float vy = 0;
  bool grounded = false;
  float coyote = 0;
  float jumpBuffer = 0;
  int facing = 1;
  int hp = 3;
  int maxHp = 3;
  int coins = 2;
  int attackPower = 1;
  bool boughtDamage = false;
  bool boughtHeart = false;
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
  int coinDrop = 3;
  float hitFlash = 0;
};

struct Checkpoint {
  bool exists = false;
  bool active = false;
  AABB box{0, 0, 20, 36};
};

struct ExitDoor {
  bool exists = false;
  bool locked = false;
  AABB box{0, 0, 24, 40};
  std::string to;
};

struct ShopStall {
  bool exists = false;
  AABB box{0, 0, 28, 36};
};

struct Sim {
  std::string assetsDir;
  Level level;
  Player player;
  Enemy enemy;
  Boss boss;
  Checkpoint checkpoint;
  ExitDoor exitDoor;
  ShopStall shop;
  ShopCatalog catalog;
  bool shopOpen = false;
  bool nearShop = false;
  float worldW = 640;
  float worldH = 288;
  std::string status;
  std::string pendingLevel;

  bool loadCatalog(std::string& error);
  bool loadFromFile(const std::string& path, std::string& error);
  bool loadNamedLevel(const std::string& fileName, std::string& error);
  void respawnPlayer();
  void fixedUpdate(float dt, const Input& input);
  bool tryBuy(std::size_t index);

  AABB attackBox() const;
  bool playerAttacking() const { return player.attackActive > 0; }
  bool itemOwned(const ShopItem& item) const;
};

}  // namespace wonderland
