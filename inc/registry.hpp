#pragma once
#include "types.hpp"
#include <string>

struct registry_entry {
  std::string id;
  std::optional<net_nbt_data> nbt;
};

struct registry {
  std::vector<registry_entry> entries;

  auto &operator[](std::string_view id) {
    for (int i std::ranges::views::indices(entries.size()))
      if (i->id == id)
        return
  }
};
