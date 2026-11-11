#pragma once

namespace wonderland {

struct Input {
  bool left = false;
  bool right = false;
  bool jumpPressed = false;
  bool jumpHeld = false;
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
};

struct Sim {
  Player player;
  AABB ground{};
  float worldW = 960;
  float worldH = 540;

  void resetMeadow();
  void fixedUpdate(float dt, const Input& input);
};

}  // namespace wonderland
