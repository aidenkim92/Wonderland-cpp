#pragma once

#include <string>
#include <vector>

namespace wonderland {

struct ShopItem {
  std::string id;
  std::string name;
  int cost = 0;
  std::string effect;  // heal | damage | max_hp
  int value = 1;
  bool once = false;
};

struct ShopCatalog {
  std::vector<ShopItem> items;
};

bool loadShopCatalog(const std::string& path, ShopCatalog& out, std::string& error);

}  // namespace wonderland
