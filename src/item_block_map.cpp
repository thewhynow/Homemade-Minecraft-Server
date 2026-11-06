
#include "inc/item_block_map.hpp"
#include "registry/blocks.hpp"
#include "registry/items.hpp"

uint16_t item_to_block(uint32_t item){
    switch(item){

        case item::acacia_planks: return block::acacia_planks;

        case item::air: return block::air;

        case item::allium: return block::allium;

        case item::amethyst_block: return block::amethyst_block;

        case item::ancient_debris: return block::ancient_debris;

        case item::andesite: return block::andesite;

        case item::azalea: return block::azalea;

        case item::azure_bluet: return block::azure_bluet;

        case item::bamboo_mosaic: return block::bamboo_mosaic;

        case item::bamboo_planks: return block::bamboo_planks;

        case item::beacon: return block::beacon;

        case item::bedrock: return block::bedrock;

        case item::birch_planks: return block::birch_planks;

        case item::black_carpet: return block::black_carpet;

        case item::black_concrete: return block::black_concrete;

        case item::black_concrete_powder: return block::black_concrete_powder;

        case item::black_stained_glass: return block::black_stained_glass;

        case item::black_terracotta: return block::black_terracotta;

        case item::black_wool: return block::black_wool;

        case item::blackstone: return block::blackstone;

        case item::blue_carpet: return block::blue_carpet;

        case item::blue_concrete: return block::blue_concrete;

        case item::blue_concrete_powder: return block::blue_concrete_powder;

        case item::blue_ice: return block::blue_ice;

        case item::blue_orchid: return block::blue_orchid;

        case item::blue_stained_glass: return block::blue_stained_glass;

        case item::blue_terracotta: return block::blue_terracotta;

        case item::blue_wool: return block::blue_wool;

        case item::bookshelf: return block::bookshelf;

        case item::brain_coral_block: return block::brain_coral_block;

        case item::bricks: return block::bricks;

        case item::brown_carpet: return block::brown_carpet;

        case item::brown_concrete: return block::brown_concrete;

        case item::brown_concrete_powder: return block::brown_concrete_powder;

        case item::brown_mushroom: return block::brown_mushroom;

        case item::brown_stained_glass: return block::brown_stained_glass;

        case item::brown_terracotta: return block::brown_terracotta;

        case item::brown_wool: return block::brown_wool;

        case item::bubble_coral_block: return block::bubble_coral_block;

        case item::budding_amethyst: return block::budding_amethyst;

        case item::bush: return block::bush;

        case item::cactus_flower: return block::cactus_flower;

        case item::calcite: return block::calcite;

        case item::cartography_table: return block::cartography_table;

        case item::cauldron: return block::cauldron;

        case item::cherry_planks: return block::cherry_planks;

        case item::chiseled_cinnabar: return block::chiseled_cinnabar;

        case item::chiseled_copper: return block::chiseled_copper;

        case item::chiseled_deepslate: return block::chiseled_deepslate;

        case item::chiseled_nether_bricks: return block::chiseled_nether_bricks;

        case item::chiseled_polished_blackstone: return block::chiseled_polished_blackstone;

        case item::chiseled_quartz_block: return block::chiseled_quartz_block;

        case item::chiseled_red_sandstone: return block::chiseled_red_sandstone;

        case item::chiseled_resin_bricks: return block::chiseled_resin_bricks;

        case item::chiseled_sandstone: return block::chiseled_sandstone;

        case item::chiseled_stone_bricks: return block::chiseled_stone_bricks;

        case item::chiseled_sulfur: return block::chiseled_sulfur;

        case item::chiseled_tuff: return block::chiseled_tuff;

        case item::chiseled_tuff_bricks: return block::chiseled_tuff_bricks;

        case item::cinnabar: return block::cinnabar;

        case item::cinnabar_bricks: return block::cinnabar_bricks;

        case item::clay: return block::clay;

        case item::closed_eyeblossom: return block::closed_eyeblossom;

        case item::coal_block: return block::coal_block;

        case item::coal_ore: return block::coal_ore;

        case item::coarse_dirt: return block::coarse_dirt;

        case item::cobbled_deepslate: return block::cobbled_deepslate;

        case item::cobblestone: return block::cobblestone;

        case item::cobweb: return block::cobweb;

        case item::copper_block: return block::copper_block;

        case item::copper_ore: return block::copper_ore;

        case item::copper_torch: return block::copper_torch;

        case item::cornflower: return block::cornflower;

        case item::cracked_deepslate_bricks: return block::cracked_deepslate_bricks;

        case item::cracked_deepslate_tiles: return block::cracked_deepslate_tiles;

        case item::cracked_nether_bricks: return block::cracked_nether_bricks;

        case item::cracked_polished_blackstone_bricks: return block::cracked_polished_blackstone_bricks;

        case item::cracked_stone_bricks: return block::cracked_stone_bricks;

        case item::crafting_table: return block::crafting_table;

        case item::crimson_fungus: return block::crimson_fungus;

        case item::crimson_nylium: return block::crimson_nylium;

        case item::crimson_planks: return block::crimson_planks;

        case item::crimson_roots: return block::crimson_roots;

        case item::crying_obsidian: return block::crying_obsidian;

        case item::cut_copper: return block::cut_copper;

        case item::cut_red_sandstone: return block::cut_red_sandstone;

        case item::cut_sandstone: return block::cut_sandstone;

        case item::cyan_carpet: return block::cyan_carpet;

        case item::cyan_concrete: return block::cyan_concrete;

        case item::cyan_concrete_powder: return block::cyan_concrete_powder;

        case item::cyan_stained_glass: return block::cyan_stained_glass;

        case item::cyan_terracotta: return block::cyan_terracotta;

        case item::cyan_wool: return block::cyan_wool;

        case item::dandelion: return block::dandelion;

        case item::dark_oak_planks: return block::dark_oak_planks;

        case item::dark_prismarine: return block::dark_prismarine;

        case item::dead_brain_coral_block: return block::dead_brain_coral_block;

        case item::dead_bubble_coral_block: return block::dead_bubble_coral_block;

        case item::dead_bush: return block::dead_bush;

        case item::dead_fire_coral_block: return block::dead_fire_coral_block;

        case item::dead_horn_coral_block: return block::dead_horn_coral_block;

        case item::dead_tube_coral_block: return block::dead_tube_coral_block;

        case item::deepslate_bricks: return block::deepslate_bricks;

        case item::deepslate_coal_ore: return block::deepslate_coal_ore;

        case item::deepslate_copper_ore: return block::deepslate_copper_ore;

        case item::deepslate_diamond_ore: return block::deepslate_diamond_ore;

        case item::deepslate_emerald_ore: return block::deepslate_emerald_ore;

        case item::deepslate_gold_ore: return block::deepslate_gold_ore;

        case item::deepslate_iron_ore: return block::deepslate_iron_ore;

        case item::deepslate_lapis_ore: return block::deepslate_lapis_ore;

        case item::deepslate_tiles: return block::deepslate_tiles;

        case item::diamond_block: return block::diamond_block;

        case item::diamond_ore: return block::diamond_ore;

        case item::diorite: return block::diorite;

        case item::dirt: return block::dirt;

        case item::dirt_path: return block::dirt_path;

        case item::dragon_egg: return block::dragon_egg;

        case item::dried_kelp_block: return block::dried_kelp_block;

        case item::dripstone_block: return block::dripstone_block;

        case item::emerald_block: return block::emerald_block;

        case item::emerald_ore: return block::emerald_ore;

        case item::enchanting_table: return block::enchanting_table;

        case item::end_stone: return block::end_stone;

        case item::end_stone_bricks: return block::end_stone_bricks;

        case item::exposed_chiseled_copper: return block::exposed_chiseled_copper;

        case item::exposed_copper: return block::exposed_copper;

        case item::exposed_cut_copper: return block::exposed_cut_copper;

        case item::fern: return block::fern;

        case item::fire_coral_block: return block::fire_coral_block;

        case item::firefly_bush: return block::firefly_bush;

        case item::fletching_table: return block::fletching_table;

        case item::flower_pot: return block::flower_pot;

        case item::flowering_azalea: return block::flowering_azalea;

        case item::frogspawn: return block::frogspawn;

        case item::gilded_blackstone: return block::gilded_blackstone;

        case item::glass: return block::glass;

        case item::glowstone: return block::glowstone;

        case item::gold_block: return block::gold_block;

        case item::gold_ore: return block::gold_ore;

        case item::golden_dandelion: return block::golden_dandelion;

        case item::granite: return block::granite;

        case item::gravel: return block::gravel;

        case item::gray_carpet: return block::gray_carpet;

        case item::gray_concrete: return block::gray_concrete;

        case item::gray_concrete_powder: return block::gray_concrete_powder;

        case item::gray_stained_glass: return block::gray_stained_glass;

        case item::gray_terracotta: return block::gray_terracotta;

        case item::gray_wool: return block::gray_wool;

        case item::green_carpet: return block::green_carpet;

        case item::green_concrete: return block::green_concrete;

        case item::green_concrete_powder: return block::green_concrete_powder;

        case item::green_stained_glass: return block::green_stained_glass;

        case item::green_terracotta: return block::green_terracotta;

        case item::green_wool: return block::green_wool;

        case item::honey_block: return block::honey_block;

        case item::honeycomb_block: return block::honeycomb_block;

        case item::horn_coral_block: return block::horn_coral_block;

        case item::ice: return block::ice;

        case item::infested_chiseled_stone_bricks: return block::infested_chiseled_stone_bricks;

        case item::infested_cobblestone: return block::infested_cobblestone;

        case item::infested_cracked_stone_bricks: return block::infested_cracked_stone_bricks;

        case item::infested_mossy_stone_bricks: return block::infested_mossy_stone_bricks;

        case item::infested_stone: return block::infested_stone;

        case item::infested_stone_bricks: return block::infested_stone_bricks;

        case item::iron_block: return block::iron_block;

        case item::iron_ore: return block::iron_ore;

        case item::jungle_planks: return block::jungle_planks;

        case item::lapis_block: return block::lapis_block;

        case item::lapis_ore: return block::lapis_ore;

        case item::light_blue_carpet: return block::light_blue_carpet;

        case item::light_blue_concrete: return block::light_blue_concrete;

        case item::light_blue_concrete_powder: return block::light_blue_concrete_powder;

        case item::light_blue_stained_glass: return block::light_blue_stained_glass;

        case item::light_blue_terracotta: return block::light_blue_terracotta;

        case item::light_blue_wool: return block::light_blue_wool;

        case item::light_gray_carpet: return block::light_gray_carpet;

        case item::light_gray_concrete: return block::light_gray_concrete;

        case item::light_gray_concrete_powder: return block::light_gray_concrete_powder;

        case item::light_gray_stained_glass: return block::light_gray_stained_glass;

        case item::light_gray_terracotta: return block::light_gray_terracotta;

        case item::light_gray_wool: return block::light_gray_wool;

        case item::lily_of_the_valley: return block::lily_of_the_valley;

        case item::lily_pad: return block::lily_pad;

        case item::lime_carpet: return block::lime_carpet;

        case item::lime_concrete: return block::lime_concrete;

        case item::lime_concrete_powder: return block::lime_concrete_powder;

        case item::lime_stained_glass: return block::lime_stained_glass;

        case item::lime_terracotta: return block::lime_terracotta;

        case item::lime_wool: return block::lime_wool;

        case item::lodestone: return block::lodestone;

        case item::magenta_carpet: return block::magenta_carpet;

        case item::magenta_concrete: return block::magenta_concrete;

        case item::magenta_concrete_powder: return block::magenta_concrete_powder;

        case item::magenta_stained_glass: return block::magenta_stained_glass;

        case item::magenta_terracotta: return block::magenta_terracotta;

        case item::magenta_wool: return block::magenta_wool;

        case item::magma_block: return block::magma_block;

        case item::mangrove_planks: return block::mangrove_planks;

        case item::melon: return block::melon;

        case item::moss_block: return block::moss_block;

        case item::moss_carpet: return block::moss_carpet;

        case item::mossy_cobblestone: return block::mossy_cobblestone;

        case item::mossy_stone_bricks: return block::mossy_stone_bricks;

        case item::mud: return block::mud;

        case item::mud_bricks: return block::mud_bricks;

        case item::nether_bricks: return block::nether_bricks;

        case item::nether_gold_ore: return block::nether_gold_ore;

        case item::nether_quartz_ore: return block::nether_quartz_ore;

        case item::nether_sprouts: return block::nether_sprouts;

        case item::nether_wart_block: return block::nether_wart_block;

        case item::netherite_block: return block::netherite_block;

        case item::netherrack: return block::netherrack;

        case item::oak_planks: return block::oak_planks;

        case item::obsidian: return block::obsidian;

        case item::open_eyeblossom: return block::open_eyeblossom;

        case item::orange_carpet: return block::orange_carpet;

        case item::orange_concrete: return block::orange_concrete;

        case item::orange_concrete_powder: return block::orange_concrete_powder;

        case item::orange_stained_glass: return block::orange_stained_glass;

        case item::orange_terracotta: return block::orange_terracotta;

        case item::orange_tulip: return block::orange_tulip;

        case item::orange_wool: return block::orange_wool;

        case item::oxeye_daisy: return block::oxeye_daisy;

        case item::oxidized_chiseled_copper: return block::oxidized_chiseled_copper;

        case item::oxidized_copper: return block::oxidized_copper;

        case item::oxidized_cut_copper: return block::oxidized_cut_copper;

        case item::packed_ice: return block::packed_ice;

        case item::packed_mud: return block::packed_mud;

        case item::pale_moss_block: return block::pale_moss_block;

        case item::pale_oak_planks: return block::pale_oak_planks;

        case item::pink_carpet: return block::pink_carpet;

        case item::pink_concrete: return block::pink_concrete;

        case item::pink_concrete_powder: return block::pink_concrete_powder;

        case item::pink_stained_glass: return block::pink_stained_glass;

        case item::pink_terracotta: return block::pink_terracotta;

        case item::pink_tulip: return block::pink_tulip;

        case item::pink_wool: return block::pink_wool;

        case item::polished_andesite: return block::polished_andesite;

        case item::polished_blackstone: return block::polished_blackstone;

        case item::polished_blackstone_bricks: return block::polished_blackstone_bricks;

        case item::polished_cinnabar: return block::polished_cinnabar;

        case item::polished_deepslate: return block::polished_deepslate;

        case item::polished_diorite: return block::polished_diorite;

        case item::polished_granite: return block::polished_granite;

        case item::polished_sulfur: return block::polished_sulfur;

        case item::polished_tuff: return block::polished_tuff;

        case item::poppy: return block::poppy;

        case item::prismarine: return block::prismarine;

        case item::prismarine_bricks: return block::prismarine_bricks;

        case item::pumpkin: return block::pumpkin;

        case item::purple_carpet: return block::purple_carpet;

        case item::purple_concrete: return block::purple_concrete;

        case item::purple_concrete_powder: return block::purple_concrete_powder;

        case item::purple_stained_glass: return block::purple_stained_glass;

        case item::purple_terracotta: return block::purple_terracotta;

        case item::purple_wool: return block::purple_wool;

        case item::purpur_block: return block::purpur_block;

        case item::quartz_block: return block::quartz_block;

        case item::quartz_bricks: return block::quartz_bricks;

        case item::raw_copper_block: return block::raw_copper_block;

        case item::raw_gold_block: return block::raw_gold_block;

        case item::raw_iron_block: return block::raw_iron_block;

        case item::red_carpet: return block::red_carpet;

        case item::red_concrete: return block::red_concrete;

        case item::red_concrete_powder: return block::red_concrete_powder;

        case item::red_mushroom: return block::red_mushroom;

        case item::red_nether_bricks: return block::red_nether_bricks;

        case item::red_sand: return block::red_sand;

        case item::red_sandstone: return block::red_sandstone;

        case item::red_stained_glass: return block::red_stained_glass;

        case item::red_terracotta: return block::red_terracotta;

        case item::red_tulip: return block::red_tulip;

        case item::red_wool: return block::red_wool;

        case item::redstone_block: return block::redstone_block;

        case item::reinforced_deepslate: return block::reinforced_deepslate;

        case item::resin_block: return block::resin_block;

        case item::resin_bricks: return block::resin_bricks;

        case item::rooted_dirt: return block::rooted_dirt;

        case item::sand: return block::sand;

        case item::sandstone: return block::sandstone;

        case item::sculk: return block::sculk;

        case item::sea_lantern: return block::sea_lantern;

        case item::seagrass: return block::seagrass;

        case item::short_dry_grass: return block::short_dry_grass;

        case item::short_grass: return block::short_grass;

        case item::shroomlight: return block::shroomlight;

        case item::slime_block: return block::slime_block;

        case item::smithing_table: return block::smithing_table;

        case item::smooth_basalt: return block::smooth_basalt;

        case item::smooth_quartz: return block::smooth_quartz;

        case item::smooth_red_sandstone: return block::smooth_red_sandstone;

        case item::smooth_sandstone: return block::smooth_sandstone;

        case item::smooth_stone: return block::smooth_stone;

        case item::snow_block: return block::snow_block;

        case item::soul_sand: return block::soul_sand;

        case item::soul_soil: return block::soul_soil;

        case item::soul_torch: return block::soul_torch;

        case item::spawner: return block::spawner;

        case item::sponge: return block::sponge;

        case item::spore_blossom: return block::spore_blossom;

        case item::spruce_planks: return block::spruce_planks;

        case item::stone: return block::stone;

        case item::stone_bricks: return block::stone_bricks;

        case item::structure_void: return block::structure_void;

        case item::sulfur: return block::sulfur;

        case item::sulfur_bricks: return block::sulfur_bricks;

        case item::tall_dry_grass: return block::tall_dry_grass;

        case item::terracotta: return block::terracotta;

        case item::test_instance_block: return block::test_instance_block;

        case item::tinted_glass: return block::tinted_glass;

        case item::torch: return block::torch;

        case item::torchflower: return block::torchflower;

        case item::tube_coral_block: return block::tube_coral_block;

        case item::tuff: return block::tuff;

        case item::tuff_bricks: return block::tuff_bricks;

        case item::warped_fungus: return block::warped_fungus;

        case item::warped_nylium: return block::warped_nylium;

        case item::warped_planks: return block::warped_planks;

        case item::warped_roots: return block::warped_roots;

        case item::warped_wart_block: return block::warped_wart_block;

        case item::waxed_chiseled_copper: return block::waxed_chiseled_copper;

        case item::waxed_copper_block: return block::waxed_copper_block;

        case item::waxed_cut_copper: return block::waxed_cut_copper;

        case item::waxed_exposed_chiseled_copper: return block::waxed_exposed_chiseled_copper;

        case item::waxed_exposed_copper: return block::waxed_exposed_copper;

        case item::waxed_exposed_cut_copper: return block::waxed_exposed_cut_copper;

        case item::waxed_oxidized_chiseled_copper: return block::waxed_oxidized_chiseled_copper;

        case item::waxed_oxidized_copper: return block::waxed_oxidized_copper;

        case item::waxed_oxidized_cut_copper: return block::waxed_oxidized_cut_copper;

        case item::waxed_weathered_chiseled_copper: return block::waxed_weathered_chiseled_copper;

        case item::waxed_weathered_copper: return block::waxed_weathered_copper;

        case item::waxed_weathered_cut_copper: return block::waxed_weathered_cut_copper;

        case item::weathered_chiseled_copper: return block::weathered_chiseled_copper;

        case item::weathered_copper: return block::weathered_copper;

        case item::weathered_cut_copper: return block::weathered_cut_copper;

        case item::wet_sponge: return block::wet_sponge;

        case item::white_carpet: return block::white_carpet;

        case item::white_concrete: return block::white_concrete;

        case item::white_concrete_powder: return block::white_concrete_powder;

        case item::white_stained_glass: return block::white_stained_glass;

        case item::white_terracotta: return block::white_terracotta;

        case item::white_tulip: return block::white_tulip;

        case item::white_wool: return block::white_wool;

        case item::wither_rose: return block::wither_rose;

        case item::yellow_carpet: return block::yellow_carpet;

        case item::yellow_concrete: return block::yellow_concrete;

        case item::yellow_concrete_powder: return block::yellow_concrete_powder;

        case item::yellow_stained_glass: return block::yellow_stained_glass;

        case item::yellow_terracotta: return block::yellow_terracotta;

        case item::yellow_wool: return block::yellow_wool;

    }

    return -1;
}
