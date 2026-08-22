#pragma once
#include "types.hpp"
#include "packet.hpp"
#include <functional>

struct registry : packet_registry_data {
    using packet_registry_data::packet_registry_data;

    template <size_t N>
    registry(
        std::string_view name,
        /* rvalue array */
        net_registry_data_entry (&&arr)[N]
    ):
        packet_registry_data{
            (uint8_t) packet_id::configuration::registry,
            name, {{}}
        }
    {
        entries().data.reserve(N);

        for (auto &&i : arr)
            entries().data.push_back(std::move(i));
    }

    const net_registry_data_entry &operator[]
        (std::string_view id) const
    ;
};

class synced_registries {
private:
    synced_registries();
public:
    void queue_packets(
        std::function<void(const packet_registry_data &)> callback
    );

    const registry &operator[]
        (std::string_view id) const
    ;

    static synced_registries instance;
private:
    registry banner_pattern,
             damage_type,
             dimension_type,
             instrument,
             jukebox_song,
             painting_variant,
             sulfur_cube_archetype,
             timeline,
             trim_material,
             world_clock,
             worldgen_biome,
             cat_variant,
             cat_sound_variant,
             chicken_variant,
             chicken_sound_variant,
             cow_variant,
             cow_sound_variant,
             frog_variant,
             pig_variant,
             pig_sound_variant,
             wolf_variant,
             wolf_sound_variant,
             zombie_nautilus_variant
    ;
};
