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

bool solidAtPx(const Level& level, float px, float py) {
  const int col =
      static_cast<int>(std::floor(px / static_cast<float>(level.tile)));
  const int row =
      static_cast<int>(std::floor(py / static_cast<float>(level.tile)));
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

AABB markerBox(const Level& level, int col, int row, float w, float h) {
  const float t = static_cast<float>(level.tile);
  AABB box{col * t + (t - w) * 0.5f, 0.0f, w, h};
  placeOnGround(level, box);
  return box;
}

bool touchingSpikes(const Level& level, const AABB& box) {
  const float t = static_cast<float>(level.tile);
  const int c0 = static_cast<int>(std::floor(box.x / t));
  const int c1 = static_cast<int>(std::floor((box.x + box.w - 1.0f) / t));
  const int r0 = static_cast<int>(std::floor(box.y / t));
  const int r1 = static_cast<int>(std::floor((box.y + box.h - 1.0f) / t));
  for (int r = r0; r <= r1; ++r) {
    for (int c = c0; c <= c1; ++c) {
      if (level.spike(c, r)) {
        return true;
      }
    }
  }
  return false;
}

void resetPlayerMotion(Player& player) {
  player.vx = 0;
  player.vy = 0;
  player.grounded = true;
  player.coyote = kCoyote;
  player.jumpBuffer = 0;
  player.attackCd = 0;
  player.attackActive = 0;
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
  in.interactPressed = IsKeyPressed(KEY_E);
  in.uiConfirm1 = IsKeyPressed(KEY_ONE) || IsKeyPressed(KEY_KP_1);
  in.uiConfirm2 = IsKeyPressed(KEY_TWO) || IsKeyPressed(KEY_KP_2);
  in.uiConfirm3 = IsKeyPressed(KEY_THREE) || IsKeyPressed(KEY_KP_3);
  in.uiClose = IsKeyPressed(KEY_ESCAPE);
  return in;
}

AABB Sim::attackBox() const {
  constexpr float w = 22.0f;
  constexpr float h = 20.0f;
  const float x = player.facing > 0 ? player.box.x + player.box.w - 4.0f
                                    : player.box.x - w + 4.0f;
  return AABB{x, player.box.y + 8.0f, w, h};
}

bool Sim::itemOwned(const ShopItem& item) const {
  if (item.id == "damage") {
    return player.boughtDamage;
  }
  if (item.id == "heart") {
    return player.boughtHeart;
  }
  return false;
}

bool Sim::loadCatalog(std::string& error) {
  if (assetsDir.empty()) {
    error = "assetsDir not set";
    return false;
  }
  return loadShopCatalog(assetsDir + "/shop/catalog.json", catalog, error);
}

bool Sim::loadNamedLevel(const std::string& fileName, std::string& error) {
  if (assetsDir.empty()) {
    error = "assetsDir not set";
    return false;
  }
  return loadFromFile(assetsDir + "/levels/" + fileName, error);
}

bool Sim::loadFromFile(const std::string& path, std::string& error) {
  Level loaded;
  if (!loadLevelFile(path, loaded, error)) {
    return false;
  }

  const int keepCoins = player.coins;
  const int keepMaxHp = player.maxHp;
  const int keepHp = player.hp;
  const int keepAtk = player.attackPower;
  const bool keepDmg = player.boughtDamage;
  const bool keepHeart = player.boughtHeart;

  level = std::move(loaded);
  worldW = static_cast<float>(level.cols * level.tile);
  worldH = static_cast<float>(level.rows * level.tile);
  pendingLevel.clear();
  shopOpen = false;
  nearShop = false;

  const float tile = static_cast<float>(level.tile);
  player.box = AABB{level.spawnCol * tile + 2.0f, 0.0f, 32.0f, 32.0f};
  placeOnGround(level, player.box);
  resetPlayerMotion(player);
  player.facing = 1;
  player.coins = keepCoins;
  player.maxHp = keepMaxHp;
  player.hp = std::clamp(keepHp, 1, keepMaxHp);
  player.attackPower = keepAtk;
  player.boughtDamage = keepDmg;
  player.boughtHeart = keepHeart;
  player.invuln = 0;

  if (level.enemy.present) {
    const float ex = level.enemy.col * tile;
    const float half =
        static_cast<float>(std::max(1, level.enemy.patrol)) * tile;
    enemy.alive = true;
    enemy.box = AABB{ex, 0.0f, 28.0f, 28.0f};
    placeOnGround(level, enemy.box);
    enemy.vx = 40.0f;
    enemy.minX = std::max(0.0f, ex - half);
    enemy.maxX = std::min(worldW, ex + half + enemy.box.w);
    enemy.hp = 2;
    enemy.coinDrop = 3;
    enemy.hitFlash = 0;
  } else {
    enemy.alive = false;
  }

  if (level.hasCheckpoint()) {
    checkpoint.exists = true;
    checkpoint.active = false;
    checkpoint.box = markerBox(level, level.checkpointCol, level.checkpointRow,
                               20.0f, 36.0f);
  } else {
    checkpoint.exists = false;
    checkpoint.active = false;
  }

  if (level.exit.valid()) {
    exitDoor.exists = true;
    exitDoor.to = level.exit.to;
    exitDoor.box =
        markerBox(level, level.exit.col, level.exit.row, 24.0f, 40.0f);
  } else {
    exitDoor.exists = false;
    exitDoor.to.clear();
  }

  if (level.boss.present) {
    boss.spawn(level, level.boss.col, level.boss.row, worldW);
    exitDoor.locked = true;
  } else {
    boss = Boss{};
    exitDoor.locked = false;
  }

  if (level.hasShop()) {
    shop.exists = true;
    shop.box = markerBox(level, level.shopCol, level.shopRow, 28.0f, 36.0f);
  } else {
    shop.exists = false;
  }

  status = level.name;
  if (boss.present) {
    status = "grove brute — hit only while stunned";
  }
  return true;
}

void Sim::respawnPlayer() {
  const float tile = static_cast<float>(level.tile);
  if (checkpoint.exists && checkpoint.active) {
    player.box = AABB{checkpoint.box.x + checkpoint.box.w * 0.5f - 16.0f, 0.0f,
                      32.0f, 32.0f};
  } else {
    player.box = AABB{level.spawnCol * tile + 2.0f, 0.0f, 32.0f, 32.0f};
  }
  placeOnGround(level, player.box);
  resetPlayerMotion(player);
  player.hp = player.maxHp;
  player.invuln = 1.0f;
  shopOpen = false;
}

bool Sim::tryBuy(std::size_t index) {
  if (index >= catalog.items.size()) {
    return false;
  }
  const ShopItem& item = catalog.items[index];
  if (item.once && itemOwned(item)) {
    status = item.name + " already owned";
    return false;
  }
  if (player.coins < item.cost) {
    status = "need more coins";
    return false;
  }
  if (item.effect == "heal" && player.hp >= player.maxHp) {
    status = "already full health";
    return false;
  }

  player.coins -= item.cost;
  if (item.effect == "heal") {
    player.hp = std::min(player.maxHp, player.hp + item.value);
    status = "bought " + item.name;
  } else if (item.effect == "damage") {
    player.attackPower += item.value;
    player.boughtDamage = true;
    status = "bought " + item.name + " (atk " +
             std::to_string(player.attackPower) + ")";
  } else if (item.effect == "max_hp") {
    player.maxHp += item.value;
    player.hp += item.value;
    player.boughtHeart = true;
    status = "bought " + item.name;
  } else {
    player.coins += item.cost;
    status = "unknown item effect";
    return false;
  }
  return true;
}

void Sim::fixedUpdate(float dt, const Input& input) {
  if (!pendingLevel.empty()) {
    return;
  }

  nearShop = shop.exists && overlap(player.box, shop.box);

  if (shopOpen) {
    player.vx = 0;
    if (input.uiClose || input.interactPressed) {
      shopOpen = false;
      status = "shop closed";
      return;
    }
    if (input.uiConfirm1) {
      tryBuy(0);
    }
    if (input.uiConfirm2) {
      tryBuy(1);
    }
    if (input.uiConfirm3) {
      tryBuy(2);
    }
    return;
  }

  if (nearShop && input.interactPressed) {
    shopOpen = true;
    status = "welcome — press 1/2/3 to buy";
    player.vx = 0;
    return;
  }

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

  if (boss.present && boss.alive) {
    boss.fixedUpdate(dt, level, player);
  }

  if (playerAttacking() && enemy.alive && overlap(attackBox(), enemy.box)) {
    enemy.hp -= player.attackPower;
    enemy.hitFlash = 0.15f;
    enemy.box.x += static_cast<float>(player.facing) * 14.0f;
    player.attackActive = 0;
    if (enemy.hp <= 0) {
      enemy.alive = false;
      player.coins += enemy.coinDrop;
      status = "blob down! +" + std::to_string(enemy.coinDrop) + " coins";
    }
  }

  if (playerAttacking() && boss.present && boss.alive &&
      overlap(attackBox(), boss.box)) {
    player.attackActive = 0;
    if (boss.tryMeleeHit(player.attackPower, player.facing)) {
      status = std::string("hit! ") + boss.stateName();
      if (!boss.alive) {
        player.coins += boss.coinDrop;
        exitDoor.locked = false;
        status = "brute down! door unlocked +" +
                 std::to_string(boss.coinDrop) + "c";
      }
    } else {
      status = "blocked — wait for stun";
    }
  }

  auto hurtPlayer = [&](const char* reason) {
    if (player.invuln > 0.0f) {
      return;
    }
    player.hp -= 1;
    player.invuln = kInvuln;
    player.vy = -120.0f;
    if (player.hp <= 0) {
      respawnPlayer();
      status = reason;
    }
  };

  if (enemy.alive && player.invuln <= 0.0f &&
      overlap(player.box, enemy.box)) {
    player.vx =
        (player.box.x + player.box.w * 0.5f < enemy.box.x + enemy.box.w * 0.5f)
            ? -kKnock
            : kKnock;
    hurtPlayer("respawned at lantern");
  }

  if (boss.present && boss.alive && player.invuln <= 0.0f &&
      overlap(player.box, boss.box)) {
    player.vx =
        (player.box.x + player.box.w * 0.5f < boss.box.x + boss.box.w * 0.5f)
            ? -kKnock
            : kKnock;
    hurtPlayer("bruised — lantern rest");
  }

  if (touchingSpikes(level, player.box)) {
    hurtPlayer("spikes — rest spot");
  }

  if (checkpoint.exists && overlap(player.box, checkpoint.box)) {
    if (!checkpoint.active) {
      checkpoint.active = true;
      player.hp = player.maxHp;
      status = "lantern lit — rest saved";
    }
  }

  if (exitDoor.exists && overlap(player.box, exitDoor.box)) {
    if (exitDoor.locked) {
      status = "door sealed until the brute falls";
    } else {
      pendingLevel = exitDoor.to;
      status = "entering " + exitDoor.to;
    }
  }

  if (player.box.y > worldH + 64.0f) {
    respawnPlayer();
    status = "fell — rest spot";
  }
}

}  // namespace wonderland
