#pragma once

#include <string>
#include <vector>

namespace wonderland {

struct LevelEnemySpawn {
  std::string type = "blob";
  int col = 0;
  int row = 0;
  int patrol = 4;
};

struct Level {
  std::string name;
  int tile = 16;
  int cols = 0;
  int rows = 0;
  int spawnCol = 0;
  int spawnRow = 0;
  LevelEnemySpawn enemy{};
  // row-major, '#' solid, '.' air
  std::vector<char> cells;

  bool solid(int col, int row) const;
  char at(int col, int row) const;
};

// Loads a constrained wonderland level JSON. Returns false on failure.
bool loadLevelFile(const std::string& path, Level& out, std::string& error);

}  // namespace wonderland
