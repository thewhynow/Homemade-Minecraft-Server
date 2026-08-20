#include "inc/registry.hpp"

synced_registries synced_registries::instance;

synced_registries::synced_registries():
    banner_pattern(
        "minecraft:banner_pattern",
        {
            {{"minecraft:pattern_item/bordure_indented"}, {nullptr}},
            {{"minecraft:pattern_item/creeper"}, {nullptr}},
            {{"minecraft:pattern_item/field_masoned"}, {nullptr}},
            {{"minecraft:pattern_item/flow"}, {nullptr}},
            {{"minecraft:pattern_item/flower"}, {nullptr}},
            {{"minecraft:pattern_item/globe"}, {nullptr}},
            {{"minecraft:pattern_item/guster"}, {nullptr}},
            {{"minecraft:pattern_item/mojang"}, {nullptr}},
            {{"minecraft:pattern_item/piglin"}, {nullptr}},
            {{"minecraft:pattern_item/skull"}, {nullptr}}
        }
    ),

    damage_type(
        "minecraft:damage_type",
        {
            {{"minecraft:cactus"}, {nullptr}},
            {{"minecraft:campfire"}, {nullptr}},
            {{"minecraft:cramming"}, {nullptr}},
            {{"minecraft:dragon_breath"}, {nullptr}},
            {{"minecraft:drown"}, {nullptr}},
            {{"minecraft:dry_out"}, {nullptr}},
            {{"minecraft:ender_pearl"}, {nullptr}},
            {{"minecraft:fall"}, {nullptr}},
            {{"minecraft:fly_into_wall"}, {nullptr}},
            {{"minecraft:freeze"}, {nullptr}},
            {{"minecraft:generic"}, {nullptr}},
            {{"minecraft:generic_kill"}, {nullptr}},
            {{"minecraft:hot_floor"}, {nullptr}},
            {{"minecraft:in_fire"}, {nullptr}},
            {{"minecraft:in_wall"}, {nullptr}},
            {{"minecraft:lava"}, {nullptr}},
            {{"minecraft:lightning_bolt"}, {nullptr}},
            {{"minecraft:magic"}, {nullptr}},
            {{"minecraft:on_fire"}, {nullptr}},
            {{"minecraft:out_of_world"}, {nullptr}},
            {{"minecraft:outside_border"}, {nullptr}},
            {{"minecraft:stalagmite"}, {nullptr}},
            {{"minecraft:starve"}, {nullptr}},
            {{"minecraft:sweet_berry_bush"}, {nullptr}},
            {{"minecraft:wither"}, {nullptr}}
        }
    ),

    dimension_type(
        "minecraft:dimension_type",
        {
            {{"minecraft:overworld"}, {nullptr}}
        }
    ),

    instrument(
        "minecraft:instrument",
        {
            {{"minecraft:ponder_goat_horn"}, {nullptr}},
        }
    ),

    jukebox_song(
        "minecraft:jukebox_song",
        {
            {{"minecraft:11"}, {nullptr}},
            {{"minecraft:13"}, {nullptr}},
            {{"minecraft:5"}, {nullptr}},
            {{"minecraft:blocks"}, {nullptr}},
            {{"minecraft:bounce"}, {nullptr}},
            {{"minecraft:cat"}, {nullptr}},
            {{"minecraft:chirp"}, {nullptr}},
            {{"minecraft:creator"}, {nullptr}},
            {{"minecraft:creator_music_box"}, {nullptr}},
            {{"minecraft:far"}, {nullptr}},
            {{"minecraft:lava_chicken"}, {nullptr}},
            {{"minecraft:mall"}, {nullptr}},
            {{"minecraft:mellohi"}, {nullptr}},
            {{"minecraft:otherside"}, {nullptr}},
            {{"minecraft:pigstep"}, {nullptr}},
            {{"minecraft:precipice"}, {nullptr}},
            {{"minecraft:relic"}, {nullptr}},
            {{"minecraft:stal"}, {nullptr}},
            {{"minecraft:strad"}, {nullptr}},
            {{"minecraft:tears"}, {nullptr}},
            {{"minecraft:wait"}, {nullptr}},
            {{"minecraft:ward"}, {nullptr}},
        }
    ),

    painting_variant(
        "minecraft:painting_variant",
        {
            {{"meditative"}, {nullptr}}
        }
    ),

    sulfur_cube_archetype(
        "minecraft:sulfur_cube_archetype",
        {
            {{"minecraft:regular"}, {nullptr}}
        }
    ),

    trim_material(
        "minecraft:trim_material",
        {
            {{"minecraft:amethyst"}, {nullptr}},
            {{"minecraft:copper"}, {nullptr}},
            {{"minecraft:diamond"}, {nullptr}},
            {{"minecraft:emerald"}, {nullptr}},
            {{"minecraft:gold"}, {nullptr}},
            {{"minecraft:iron"}, {nullptr}},
            {{"minecraft:lapis"}, {nullptr}},
            {{"minecraft:netherite"}, {nullptr}},
            {{"minecraft:quartz"}, {nullptr}},
            {{"minecraft:redstone"}, {nullptr}},
            {{"minecraft:resin"}, {nullptr}},
        }
    ),

    worldgen_biome(
        "minecraft:worldgen/biome",
        {
            {{"minecraft:plains"}, {nullptr}},
        }
    ),

    cat_variant(
        "minecraft:cat_variant",
        {
            {{"white"}, {nullptr}},
        }
    ),

    cat_sound_variant(
        "minecraft:cat_sound_variant",
        {
            {{"minecraft:classic"}, {nullptr}},
        }
    ),

    chicken_variant(
        "minecraft:chicken_variant",
        {
            {{"minecraft:cold"}, {nullptr}},
            {{"minecraft:temperate"}, {nullptr}},
            {{"minecraft:warm"}, {nullptr}},
        }
    ),

    chicken_sound_variant(
        "minecraft:chicken_sound_variant",
        {
            {{"classic"}, {nullptr}},
        }
    ),

    cow_variant(
        "minecraft:cow_variant",
        {
            {{"minecraft:temperate"}, {nullptr}},
        }
    ),

    cow_sound_variant(
        "minecraft:cow_sound_variant",
        {
            {{"minecraft:classic"}, {nullptr}},
        }
    ),

    frog_variant(
        "minecraft:frog_variant",
        {
            {{"minecraft:temperate"}, {nullptr}},
        }
    ),

    pig_variant(
        "minecraft:pig_variant",
        {
            {{"minecraft:temperate"}, {nullptr}},
        }
    ),

    pig_sound_variant(
        "minecraft:pig_sound_variant",
        {
            {{"minecraft:classic"}, {nullptr}},
        }
    ),

    wolf_variant(
        "minecraft:wolf_variant",
        {
            {{"minecraft:pale"}, {nullptr}},
        }
    ),

    wolf_sound_variant(
        "minecraft:wolf_sound_variant",
        {
            {{"minecraft:classic"}, {nullptr}},
        }
    ),

    zombie_nautilus_variant(
        "minecraft:zombie_nautilus_variant",
        {
            {{"minecraft:temperate"}, {nullptr}},
        }
    )
{}

void synced_registries::queue_packets(
    std::function<void(const packet_registry_data &)> callback
){
    callback(banner_pattern);
    callback(damage_type);
    callback(dimension_type);
    callback(instrument);
    callback(jukebox_song);
    callback(painting_variant);
    callback(sulfur_cube_archetype);
    callback(trim_material);
    callback(worldgen_biome);
    callback(cat_variant);
    callback(cat_sound_variant);
    callback(chicken_variant);
    callback(chicken_sound_variant);
    callback(cow_variant);
    callback(cow_sound_variant);
    callback(frog_variant);
    callback(pig_variant);
    callback(pig_sound_variant);
    callback(wolf_variant);
    callback(wolf_sound_variant);
    callback(zombie_nautilus_variant);
}
