#include "inc/registry.hpp"

int registry::operator[]
    (std::string_view id) const
{
    for (size_t i = 0; i < entries().data.size(); ++i)
        if (id == entries().data[i].id().value)
            return i;

    throw std::out_of_range("bad id");
}

const registry &synced_registries::operator[]
    (std::string_view id) const
{
    if (id == "banner_pattern")
        return banner_pattern;

    if (id == "damage_type")
        return damage_type;

    if (id == "dimension_type")
        return dimension_type;

    if (id == "instrument")
        return instrument;

    if (id == "jukebox_song")
        return jukebox_song;

    if (id == "painting_variant")
        return painting_variant;

    if (id == "sulfur_cube_archetype")
        return sulfur_cube_archetype;

    if (id == "timeline")
        return timeline;

    if (id == "trim_material")
        return trim_material;

    if (id == "world_clock")
        return world_clock;

    if (id == "worldgen_biome")
        return worldgen_biome;

    if (id == "cat_variant")
        return cat_variant;

    if (id == "cat_sound_variant")
        return cat_sound_variant;

    if (id == "chicken_variant")
        return chicken_variant;

    if (id == "chicken_sound_variant")
        return chicken_sound_variant;

    if (id == "cow_variant")
        return cow_variant;

    if (id == "cow_sound_variant")
        return cow_sound_variant;

    if (id == "frog_variant")
        return frog_variant;

    if (id == "pig_variant")
        return pig_variant;

    if (id == "pig_sound_variant")
        return pig_sound_variant;

    if (id == "wolf_variant")
        return wolf_variant;

    if (id == "wolf_sound_variant")
        return wolf_sound_variant;

    if (id == "zombie_nautilus_variant")
        return zombie_nautilus_variant;

    throw std::out_of_range("bad registry");
}

synced_registries synced_registries::instance;

synced_registries::synced_registries():
    banner_pattern(
        "minecraft:banner_pattern",
        {
            {{"minecraft:base"}, {nullptr}},
            {{"minecraft:border"}, {nullptr}},
            {{"minecraft:bricks"}, {nullptr}},
            {{"minecraft:circle"}, {nullptr}},
            {{"minecraft:creeper"}, {nullptr}},
            {{"minecraft:cross"}, {nullptr}},
            {{"minecraft:curly_border"}, {nullptr}},
            {{"minecraft:diagonal_left"}, {nullptr}},
            {{"minecraft:diagonal_right"}, {nullptr}},
            {{"minecraft:diagonal_up_left"}, {nullptr}},
            {{"minecraft:diagonal_up_right"}, {nullptr}},
            {{"minecraft:flow"}, {nullptr}},
            {{"minecraft:flower"}, {nullptr}},
            {{"minecraft:globe"}, {nullptr}},
            {{"minecraft:gradient"}, {nullptr}},
            {{"minecraft:gradient_up"}, {nullptr}},
            {{"minecraft:guster"}, {nullptr}},
            {{"minecraft:half_horizontal"}, {nullptr}},
            {{"minecraft:half_horizontal_bottom"}, {nullptr}},
            {{"minecraft:half_vertical"}, {nullptr}},
            {{"minecraft:half_vertical_right"}, {nullptr}},
            {{"minecraft:mojang"}, {nullptr}},
            {{"minecraft:piglin"}, {nullptr}},
            {{"minecraft:rhombus"}, {nullptr}},
            {{"minecraft:skull"}, {nullptr}},
            {{"minecraft:small_stripes"}, {nullptr}},
            {{"minecraft:square_bottom_left"}, {nullptr}},
            {{"minecraft:square_bottom_right"}, {nullptr}},
            {{"minecraft:square_top_left"}, {nullptr}},
            {{"minecraft:square_top_right"}, {nullptr}},
            {{"minecraft:straight_cross"}, {nullptr}},
            {{"minecraft:stripe_bottom"}, {nullptr}},
            {{"minecraft:stripe_center"}, {nullptr}},
            {{"minecraft:stripe_downleft"}, {nullptr}},
            {{"minecraft:stripe_downright"}, {nullptr}},
            {{"minecraft:stripe_left"}, {nullptr}},
            {{"minecraft:stripe_middle"}, {nullptr}},
            {{"minecraft:stripe_right"}, {nullptr}},
            {{"minecraft:stripe_top"}, {nullptr}},
            {{"minecraft:triangle_bottom"}, {nullptr}},
            {{"minecraft:triangle_top"}, {nullptr}},
            {{"minecraft:triangles_bottom"}, {nullptr}},
            {{"minecraft:triangles_top"}, {nullptr}}
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
            {{"minecraft:spear"}, {nullptr}},
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
            {{"minecraft:meditative"}, {nullptr}}
        }
    ),

    sulfur_cube_archetype(
        "minecraft:sulfur_cube_archetype",
        {
            {{"minecraft:regular"}, {nullptr}}
        }
    ),

    timeline(
        "minecraft:timeline",
        {
            {{"minecraft:day"}, {nullptr}},
            {{"minecraft:early_game"}, {nullptr}},
            {{"minecraft:moon"}, {nullptr}},
            {{"minecraft:villager_schedule"}, {nullptr}}
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

    world_clock(
        "minecraft:world_clock",
        {
            {{"minecraft:overworld"}, {nullptr}},
            {{"minecraft:the_end"}, {nullptr}}
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
            {{"minecraft:white"}, {nullptr}},
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
            {{"minecraft:classic"}, {nullptr}},
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
    callback(timeline);
    callback(trim_material);
    callback(world_clock);
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
