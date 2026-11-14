#pragma once

#include <string>
#include <vector>

namespace wonderland {

struct LevelEnemySpawn {
  std::string type = "blob";
  int col = 0;
  int row = 0;
  int patrol = 4;
  bool present = true;
};

struct LevelBossSpawn {
  std::string type = "grove_brute";
  int col = 0;
  int row = 0;
  bool present = false;
};

struct LevelExit {
  int col = -1;
  int row = -1;
  std::string to;
  bool valid() const { return col >= 0 && row >= 0 && !to.empty(); }
};

struct Level {
  std::string name;
  std::string theme = "meadow";
  int tile = 16;
  int cols = 0;
  int rows = 0;
  int spawnCol = 0;
  int spawnRow = 0;
  int checkpointCol = -1;
  int checkpointRow = -1;
  LevelExit exit{};
  LevelEnemySpawn enemy{};
  LevelBossSpawn boss{};
  int shopCol = -1;
  int shopRow = -1;
  // row-major: '#' solid, '^' spike, '.' air
  std::vector<char> cells;

  bool solid(int col, int row) const;
  bool spike(int col, int row) const;
  char at(int col, int row) const;
  bool hasCheckpoint() const {
    return checkpointCol >= 0 && checkpointRow >= 0;
  }
  bool hasShop() const { return shopCol >= 0 && shopRow >= 0; }
};

bool loadLevelFile(const std::string& path, Level& out, std::string& error);

}  // namespace wonderland
