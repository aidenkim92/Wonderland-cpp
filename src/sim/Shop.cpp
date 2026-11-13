#include "sim/Shop.hpp"

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

std::string stringField(const std::string& block, const std::string& key) {
  const auto pos = block.find("\"" + key + "\"");
  if (pos == std::string::npos) {
    return {};
  }
  const auto colon = block.find(':', pos);
  const auto q1 = block.find('"', colon + 1);
  const auto q2 = block.find('"', q1 + 1);
  if (colon == std::string::npos || q1 == std::string::npos ||
      q2 == std::string::npos) {
    return {};
  }
  return block.substr(q1 + 1, q2 - q1 - 1);
}

bool intField(const std::string& block, const std::string& key, int& out) {
  const auto pos = block.find("\"" + key + "\"");
  if (pos == std::string::npos) {
    return false;
  }
  const auto colon = block.find(':', pos);
  if (colon == std::string::npos) {
    return false;
  }
  std::size_t i = colon + 1;
  while (i < block.size() &&
         std::isspace(static_cast<unsigned char>(block[i]))) {
    ++i;
  }
  if (i >= block.size() || !std::isdigit(static_cast<unsigned char>(block[i]))) {
    return false;
  }
  int value = 0;
  while (i < block.size() &&
         std::isdigit(static_cast<unsigned char>(block[i]))) {
    value = value * 10 + (block[i] - '0');
    ++i;
  }
  out = value;
  return true;
}

bool boolField(const std::string& block, const std::string& key, bool& out) {
  const auto pos = block.find("\"" + key + "\"");
  if (pos == std::string::npos) {
    return false;
  }
  const auto colon = block.find(':', pos);
  if (colon == std::string::npos) {
    return false;
  }
  const auto limit = block.find_first_of(",}", colon + 1);
  const auto region = block.substr(colon + 1, limit - colon - 1);
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

bool loadShopCatalog(const std::string& path, ShopCatalog& out,
                     std::string& error) {
  const std::string src = readFile(path, error);
  if (src.empty()) {
    return false;
  }

  const auto itemsKey = src.find("\"items\"");
  if (itemsKey == std::string::npos) {
    error = "missing items array";
    return false;
  }
  const auto lb = src.find('[', itemsKey);
  const auto rb = src.find(']', lb);
  if (lb == std::string::npos || rb == std::string::npos) {
    error = "malformed items array";
    return false;
  }

  ShopCatalog catalog;
  std::size_t search = lb + 1;
  while (search < rb) {
    const auto brace = src.find('{', search);
    if (brace == std::string::npos || brace >= rb) {
      break;
    }
    const auto end = src.find('}', brace);
    if (end == std::string::npos || end > rb) {
      error = "unclosed shop item";
      return false;
    }
    const std::string block = src.substr(brace, end - brace + 1);
    search = end + 1;

    ShopItem item;
    item.id = stringField(block, "id");
    item.name = stringField(block, "name");
    item.effect = stringField(block, "effect");
    if (item.id.empty() || item.effect.empty()) {
      continue;
    }
    intField(block, "cost", item.cost);
    intField(block, "value", item.value);
    boolField(block, "once", item.once);
    catalog.items.push_back(std::move(item));
  }

  if (catalog.items.empty()) {
    error = "no shop items parsed";
    return false;
  }
  out = std::move(catalog);
  return true;
}

}  // namespace wonderland
