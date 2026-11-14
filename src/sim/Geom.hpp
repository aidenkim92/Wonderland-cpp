#pragma once

namespace wonderland {

struct AABB {
  float x = 0;
  float y = 0;
  float w = 0;
  float h = 0;
};

inline bool overlap(const AABB& a, const AABB& b) {
  return a.x < b.x + b.w && a.x + a.w > b.x && a.y < b.y + b.h &&
         a.y + a.h > b.y;
}

}  // namespace wonderland
