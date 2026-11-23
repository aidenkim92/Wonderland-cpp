#pragma once

#include <string>

namespace wonderland {

struct SaveData {
  std::string level = "meadow_01.json";
  int coins = 2;
  int maxHp = 3;
  int hp = 3;
  int attackPower = 1;
  bool boughtDamage = false;
  bool boughtHeart = false;
  bool checkpointActive = false;
};

std::string defaultSavePath();
bool saveExists(const std::string& path);
bool writeSave(const std::string& path, const SaveData& data, std::string& error);
bool readSave(const std::string& path, SaveData& data, std::string& error);

}  // namespace wonderland
