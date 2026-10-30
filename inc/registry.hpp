#pragma once
#include "types.hpp"
#include "packet.hpp"
#include <functional>

struct registry : packet_registry_data {
    using packet_registry_data::packet_registry_data;

    std::optional<net_registry_data_entry> default_value;

    template <size_t N>
    registry(
        std::string_view name,
        /* rvalue array */
        net_registry_data_entry (&&arr)[N],
        std::optional<net_registry_data_entry> &&default_value = std::nullopt
    ):
        packet_registry_data{
            name, {{}}
        },
        default_value(std::move(default_value))
    {
        entries().data.reserve(N);

        for (auto &&i : arr)
            entries().data.push_back(std::move(i));
    }

    int operator[] (std::string_view id) const;
    std::string_view operator[] (int id) const;
};

class registry_manager {
private:
    registry_manager();
public:
    static registry_manager instance;

    const registry &operator[] (std::string_view id) const;

    void queue_synced_registries_packets (
        std::function<void(const packet_registry_data &)> callback
    );

private:
    std::array<registry, 118> registries;
};
