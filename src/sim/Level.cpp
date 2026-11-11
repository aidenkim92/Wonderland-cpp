#include "sim/Level.hpp"

#include <cctype>
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
  if (q1 == std::string::npos) {
    return false;
  }
  const auto q2 = src.find('"', q1 + 1);
  if (q2 == std::string::npos) {
    return false;
  }
  out = src.substr(q1 + 1, q2 - q1 - 1);
  return true;
}

bool parseIntPairAfter(const std::string& src, const std::string& key, int& a,
                       int& b) {
  const auto pos = src.find("\"" + key + "\"");
  if (pos == std::string::npos) {
    return false;
  }
  const auto lb = src.find('[', pos);
  const auto rb = src.find(']', lb);
  if (lb == std::string::npos || rb == std::string::npos) {
    return false;
  }
  const std::string inside = src.substr(lb + 1, rb - lb - 1);
  std::istringstream iss(inside);
  char comma = 0;
  if (!(iss >> a >> comma >> b) || comma != ',') {
    return false;
  }
  return true;
}

bool parseTiles(const std::string& src, int cols, int rows,
                std::vector<char>& cells, std::string& error) {
  const auto key = src.find("\"tiles\"");
  if (key == std::string::npos) {
    error = "missing tiles";
    return false;
  }
  const auto lb = src.find('[', key);
  const auto rb = src.rfind(']');
  if (lb == std::string::npos || rb == std::string::npos || rb <= lb) {
    error = "malformed tiles array";
    return false;
  }

  cells.assign(static_cast<std::size_t>(cols * rows), '.');
  int row = 0;
  std::size_t i = lb + 1;
  while (i < rb && row < rows) {
    const auto q1 = src.find('"', i);
    if (q1 == std::string::npos || q1 >= rb) {
      break;
    }
    const auto q2 = src.find('"', q1 + 1);
    if (q2 == std::string::npos || q2 >= rb) {
      error = "unclosed tile row string";
      return false;
    }
    const std::string line = src.substr(q1 + 1, q2 - q1 - 1);
    if (static_cast<int>(line.size()) != cols) {
      error = "tile row width mismatch at row " + std::to_string(row);
      return false;
    }
    for (int c = 0; c < cols; ++c) {
      const char ch = line[static_cast<std::size_t>(c)];
      cells[static_cast<std::size_t>(row * cols + c)] =
          (ch == '#') ? '#' : '.';
    }
    ++row;
    i = q2 + 1;
  }
  if (row != rows) {
    error = "tile row count mismatch";
    return false;
  }
  return true;
}

}  // namespace

bool Level::solid(int col, int row) const {
  return at(col, row) == '#';
}

char Level::at(int col, int row) const {
  if (col < 0 || row < 0 || col >= cols || row >= rows) {
    return '#';
  }
  return cells[static_cast<std::size_t>(row * cols + col)];
}

bool loadLevelFile(const std::string& path, Level& out, std::string& error) {
  const std::string src = readFile(path, error);
  if (src.empty()) {
    return false;
  }

  Level level;
  if (!parseStringAfter(src, "name", level.name)) {
    level.name = "unnamed";
  }
  if (!parseIntAfter(src, "tile", level.tile) || level.tile <= 0) {
    error = "invalid tile size";
    return false;
  }
  if (!parseIntAfter(src, "cols", level.cols) ||
      !parseIntAfter(src, "rows", level.rows) || level.cols <= 0 ||
      level.rows <= 0) {
    error = "invalid cols/rows";
    return false;
  }
  if (!parseIntPairAfter(src, "spawn", level.spawnCol, level.spawnRow)) {
    error = "invalid spawn";
    return false;
  }

  {
    const auto enemyPos = src.find("\"enemy\"");
    if (enemyPos != std::string::npos) {
      const auto brace = src.find('{', enemyPos);
      const auto end = src.find('}', brace);
      if (brace != std::string::npos && end != std::string::npos) {
        const std::string block = src.substr(brace, end - brace + 1);
        parseStringAfter(block, "type", level.enemy.type);
        parseIntAfter(block, "col", level.enemy.col);
        parseIntAfter(block, "row", level.enemy.row);
        parseIntAfter(block, "patrol", level.enemy.patrol);
      }
    } else {
      level.enemy.col = level.spawnCol + 8;
      level.enemy.row = level.spawnRow;
    }
  }

  if (!parseTiles(src, level.cols, level.rows, level.cells, error)) {
    return false;
  }

  out = std::move(level);
  return true;
}

}  // namespace wonderland
