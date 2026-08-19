#include "inc/registry.hpp"
#include "inc/packet.hpp"
#include "inc/types.hpp"
#include <vector>

registry_entry::operator net_registry_data_entry() && {
    return net_registry_data_entry(
        {std::move(id)},
        {std::move(*nbt)}
    );
}

int registry::operator[](std::string_view id) const {
    for (size_t i = 0; i < entries.size(); ++i)
        if (entries[i].id == id)
            return i;

    throw std::out_of_range("invalid id");
}

registry::operator packet_registry_data() && {
    std::vector<net_registry_data_entry> new_entries;
    entries.reserve(entries.size());

    for (auto &&i : std::move(entries))
        new_entries.push_back((net_registry_data_entry &&) i);

    return packet_registry_data(
        (uint8_t) packet_id::configuration::registry,
        {std::move(name)},
        {std::move(new_entries)}
    );
}
