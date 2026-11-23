#include "sim/Save.hpp"

#include <cctype>
#include <cstdlib>
#include <fstream>
#include <sstream>

namespace wonderland {
namespace {

std::string readFile(const std::string& path, std::string& error) {
  std::ifstream in(path);
  if (!in) {
    error = "could not open " + path;
    return {};
  }
  std::ostringstream ss;
  ss << in.rdbuf();
  return ss.str();
}

bool parseIntAfter(const std::string& src, const std::string& key, int& out) {
  const auto pos = src.find("\"" + key + "\"");
  if (pos == std::string::npos) {
    return false;
  }
  const auto colon = src.find(':', pos);
  if (colon == std::string::npos) {
    return false;
  }
  std::size_t i = colon + 1;
  while (i < src.size() && std::isspace(static_cast<unsigned char>(src[i]))) {
    ++i;
  }
  if (i >= src.size()) {
    return false;
  }
  const bool neg = src[i] == '-';
  if (neg) {
    ++i;
  }
  if (i >= src.size() || !std::isdigit(static_cast<unsigned char>(src[i]))) {
    return false;
  }
  int value = 0;
  while (i < src.size() && std::isdigit(static_cast<unsigned char>(src[i]))) {
    value = value * 10 + (src[i] - '0');
    ++i;
  }
  out = neg ? -value : value;
  return true;
}

bool parseStringAfter(const std::string& src, const std::string& key,
                      std::string& out) {
  const auto pos = src.find("\"" + key + "\"");
  if (pos == std::string::npos) {
    return false;
  }
  const auto colon = src.find(':', pos);
  if (colon == std::string::npos) {
    return false;
  }
  const auto q1 = src.find('"', colon + 1);
  const auto q2 = src.find('"', q1 + 1);
  if (q1 == std::string::npos || q2 == std::string::npos) {
    return false;
  }
  out = src.substr(q1 + 1, q2 - q1 - 1);
  return true;
}

bool parseBoolAfter(const std::string& src, const std::string& key, bool& out) {
  const auto pos = src.find("\"" + key + "\"");
  if (pos == std::string::npos) {
    return false;
  }
  const auto colon = src.find(':', pos);
  if (colon == std::string::npos) {
    return false;
  }
  const auto limit = src.find_first_of(",}", colon + 1);
  const auto region = src.substr(colon + 1, limit - colon - 1);
  if (region.find("true") != std::string::npos) {
    out = true;
    return true;
  }
  if (region.find("false") != std::string::npos) {
    out = false;
    return true;
  }
  return false;
}

}  // namespace

std::string defaultSavePath() {
  if (const char* home = std::getenv("HOME")) {
    return std::string(home) + "/.wonderland_save.json";
  }
  return "wonderland_save.json";
}

bool saveExists(const std::string& path) {
  std::ifstream in(path);
  return static_cast<bool>(in);
}

bool writeSave(const std::string& path, const SaveData& data,
               std::string& error) {
  std::ofstream out(path);
  if (!out) {
    error = "could not write " + path;
    return false;
  }
  out << "{\n"
      << "  \"level\": \"" << data.level << "\",\n"
      << "  \"coins\": " << data.coins << ",\n"
      << "  \"max_hp\": " << data.maxHp << ",\n"
      << "  \"hp\": " << data.hp << ",\n"
      << "  \"attack\": " << data.attackPower << ",\n"
      << "  \"bought_damage\": " << (data.boughtDamage ? "true" : "false")
      << ",\n"
      << "  \"bought_heart\": " << (data.boughtHeart ? "true" : "false") << ",\n"
      << "  \"checkpoint\": " << (data.checkpointActive ? "true" : "false")
      << "\n"
      << "}\n";
  if (!out) {
    error = "write failed for " + path;
    return false;
  }
  return true;
}

bool readSave(const std::string& path, SaveData& data, std::string& error) {
  const std::string src = readFile(path, error);
  if (src.empty()) {
    return false;
  }

  SaveData loaded;
  if (!parseStringAfter(src, "level", loaded.level) || loaded.level.empty()) {
    error = "save missing level";
    return false;
  }
  parseIntAfter(src, "coins", loaded.coins);
  parseIntAfter(src, "max_hp", loaded.maxHp);
  parseIntAfter(src, "hp", loaded.hp);
  parseIntAfter(src, "attack", loaded.attackPower);
  parseBoolAfter(src, "bought_damage", loaded.boughtDamage);
  parseBoolAfter(src, "bought_heart", loaded.boughtHeart);
  parseBoolAfter(src, "checkpoint", loaded.checkpointActive);

  if (loaded.maxHp < 1) {
    loaded.maxHp = 3;
  }
  if (loaded.hp < 1) {
    loaded.hp = loaded.maxHp;
  }
  if (loaded.hp > loaded.maxHp) {
    loaded.hp = loaded.maxHp;
  }
  if (loaded.attackPower < 1) {
    loaded.attackPower = 1;
  }
  if (loaded.coins < 0) {
    loaded.coins = 0;
  }

  data = std::move(loaded);
  return true;
}

}  // namespace wonderland
