#pragma once
#include "types.hpp"
#include "packet.hpp"
#include <string>
#include <optional>

struct registry_entry {
    std::string id;
    std::optional<net_nbt_data> nbt;

    operator net_registry_data_entry() &&;
};

struct registry {
    std::string name;
    std::vector<registry_entry> entries;

    int operator[](std::string_view id) const;

    operator packet_registry_data() &&;
};

class synced_registries {
private:
    synced_registries();
public:
    const std::vector<packet_registry_data> &get();
private:
    registry_entry e {
        "a", std::nullopt
    };

    registry banner_pattern {
        "minecraft:banner_pattern",
        {
            {"minecraft:pattern_item/bordure_indented", std::nullopt},
        }
    };
};
