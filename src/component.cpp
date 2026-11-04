#include "inc/component.hpp"
#include "registry/components.hpp"


structured_component_lodestone_tracker::structured_component_lodestone_tracker(
    std::span<uint8_t> &buff
):
    has_global_position(buff),
    dimension(
        has_global_position
            ? std::optional<net_identifier>(std::in_place, buff)
            : std::nullopt
    ),
    position(
        has_global_position
            ? std::optional<net_position>(std::in_place, buff)
            : std::nullopt
    ),
    tracked(buff)
{}

void structured_component_lodestone_tracker::serialize(
    std::vector<uint8_t> &buff
) const {
    has_global_position.serialize(buff);

    if (has_global_position) {
        dimension->serialize(buff);
        position->serialize(buff);
    }

    tracked.serialize(buff);
}

size_t structured_component_lodestone_tracker::size() const {
    size_t res = has_global_position.size();

    if (has_global_position) {
        res += dimension->size();
        res += position->size();
    }

    return res + tracked.size();
}

std::unique_ptr<structured_component_base>
    structured_component_base::build_structured_component(
        net_var_int type, std::span<uint8_t> &buff
    )
{
    switch(type){
        case component::additional_trade_cost:
            return std::unique_ptr<structured_component_base> {
                new structured_component_additional_trade_cost {buff}
            };

        case component::attack_range:
            return std::unique_ptr<structured_component_base> {
                new structured_component_attack_range {buff}
            };

        case component::attribute_modifiers:
            return std::unique_ptr<structured_component_base> {
                new structured_component_attribute_modifiers {buff}
            };

        case component::axolotl_variant:
            return std::unique_ptr<structured_component_base> {
                new structured_component_axolotl_variant {buff}
            };

        case component::banner_patterns:
            return std::unique_ptr<structured_component_base> {
                new structured_component_banner_patterns {buff}
            };

        case component::base_color:
            return std::unique_ptr<structured_component_base> {
                new structured_component_base_color {buff}
            };

        case component::bees:
            return std::unique_ptr<structured_component_base> {
                new structured_component_bees {buff}
            };

        case component::block_entity_data:
            return std::unique_ptr<structured_component_base> {
                new structured_component_block_entity_data {buff}
            };

        case component::block_state:
            return std::unique_ptr<structured_component_base> {
                new structured_component_block_state {buff}
            };

        case component::blocks_attacks:
            return std::unique_ptr<structured_component_base> {
                new structured_component_blocks_attacks {buff}
            };

        case component::break_sound:
            return std::unique_ptr<structured_component_base> {
                new structured_component_break_sound {buff}
            };

        case component::bucket_entity_data:
            return std::unique_ptr<structured_component_base> {
                new structured_component_bucket_entity_data {buff}
            };

        case component::bundle_contents:
            return std::unique_ptr<structured_component_base> {
                new structured_component_bundle_contents {buff}
            };

        case component::can_break:
            return std::unique_ptr<structured_component_base> {
                new structured_component_can_break {buff}
            };

        case component::can_place_on:
            return std::unique_ptr<structured_component_base> {
                new structured_component_can_place_on {buff}
            };

        case component::cat_collar:
            return std::unique_ptr<structured_component_base> {
                new structured_component_cat_collar {buff}
            };

        case component::cat_sound_variant:
            return std::unique_ptr<structured_component_base> {
                new structured_component_cat_sound_variant {buff}
            };

        case component::cat_variant:
            return std::unique_ptr<structured_component_base> {
                new structured_component_cat_variant {buff}
            };

        case component::charged_projectiles:
            return std::unique_ptr<structured_component_base> {
                new structured_component_charged_projectiles {buff}
            };

        case component::chicken_sound_variant:
            return std::unique_ptr<structured_component_base> {
                new structured_component_chicken_sound_variant {buff}
            };

        case component::chicken_variant:
            return std::unique_ptr<structured_component_base> {
                new structured_component_chicken_variant {buff}
            };

        case component::consumable:
            return std::unique_ptr<structured_component_base> {
                new structured_component_consumable {buff}
            };

        case component::container:
            return std::unique_ptr<structured_component_base> {
                new structured_component_container {buff}
            };

        case component::container_loot:
            return std::unique_ptr<structured_component_base> {
                new structured_component_container_loot {buff}
            };

        case component::cow_sound_variant:
            return std::unique_ptr<structured_component_base> {
                new structured_component_cow_sound_variant {buff}
            };

        case component::cow_variant:
            return std::unique_ptr<structured_component_base> {
                new structured_component_cow_variant {buff}
            };

        case component::creative_slot_lock:
            return std::unique_ptr<structured_component_base> {
                new structured_component_creative_slot_lock {buff}
            };

        case component::custom_data:
            return std::unique_ptr<structured_component_base> {
                new structured_component_custom_data {buff}
            };

        case component::custom_model_data:
            return std::unique_ptr<structured_component_base> {
                new structured_component_custom_model_data {buff}
            };

        case component::custom_name:
            return std::unique_ptr<structured_component_base> {
                new structured_component_custom_name {buff}
            };

        case component::damage:
            return std::unique_ptr<structured_component_base> {
                new structured_component_damage {buff}
            };

        case component::damage_resistant:
            return std::unique_ptr<structured_component_base> {
                new structured_component_damage_resistant {buff}
            };

        case component::damage_type:
            return std::unique_ptr<structured_component_base> {
                new structured_component_damage_type {buff}
            };

        case component::death_protection:
            return std::unique_ptr<structured_component_base> {
                new structured_component_death_protection {buff}
            };

        case component::debug_stick_state:
            return std::unique_ptr<structured_component_base> {
                new structured_component_debug_stick_state {buff}
            };

        case component::dye:
            return std::unique_ptr<structured_component_base> {
                new structured_component_dye {buff}
            };

        case component::dyed_color:
            return std::unique_ptr<structured_component_base> {
                new structured_component_dyed_color {buff}
            };

        case component::enchantable:
            return std::unique_ptr<structured_component_base> {
                new structured_component_enchantable {buff}
            };

        case component::enchantment_glint_override:
            return std::unique_ptr<structured_component_base> {
                new structured_component_enchantment_glint_override {buff}
            };

        case component::enchantments:
            return std::unique_ptr<structured_component_base> {
                new structured_component_enchantments {buff}
            };

        case component::entity_data:
            return std::unique_ptr<structured_component_base> {
                new structured_component_entity_data {buff}
            };

        case component::equippable:
            return std::unique_ptr<structured_component_base> {
                new structured_component_equippable {buff}
            };

        case component::firework_explosion:
            return std::unique_ptr<structured_component_base> {
                new structured_component_firework_explosion {buff}
            };

        case component::fireworks:
            return std::unique_ptr<structured_component_base> {
                new structured_component_fireworks {buff}
            };

        case component::food:
            return std::unique_ptr<structured_component_base> {
                new structured_component_food {buff}
            };

        case component::fox_variant:
            return std::unique_ptr<structured_component_base> {
                new structured_component_fox_variant {buff}
            };

        case component::frog_variant:
            return std::unique_ptr<structured_component_base> {
                new structured_component_frog_variant {buff}
            };

        case component::glider:
            return std::unique_ptr<structured_component_base> {
                new structured_component_glider {buff}
            };

        case component::horse_variant:
            return std::unique_ptr<structured_component_base> {
                new structured_component_horse_variant {buff}
            };

        case component::instrument:
            return std::unique_ptr<structured_component_base> {
                new structured_component_instrument {buff}
            };

        case component::intangible_projectile:
            return std::unique_ptr<structured_component_base> {
                new structured_component_intangible_projectile {buff}
            };

        case component::item_model:
            return std::unique_ptr<structured_component_base> {
                new structured_component_item_model {buff}
            };

        case component::item_name:
            return std::unique_ptr<structured_component_base> {
                new structured_component_item_name {buff}
            };

        case component::jukebox_playable:
            return std::unique_ptr<structured_component_base> {
                new structured_component_jukebox_playable {buff}
            };

        case component::kinetic_weapon:
            return std::unique_ptr<structured_component_base> {
                new structured_component_kinetic_weapon {buff}
            };

        case component::llama_variant:
            return std::unique_ptr<structured_component_base> {
                new structured_component_llama_variant {buff}
            };

        case component::lock:
            return std::unique_ptr<structured_component_base> {
                new structured_component_lock {buff}
            };

        case component::lodestone_tracker:
            return std::unique_ptr<structured_component_base> {
                new structured_component_lodestone_tracker {buff}
            };

        case component::lore:
            return std::unique_ptr<structured_component_base> {
                new structured_component_lore {buff}
            };

        case component::map_color:
            return std::unique_ptr<structured_component_base> {
                new structured_component_map_color {buff}
            };

        case component::map_decorations:
            return std::unique_ptr<structured_component_base> {
                new structured_component_map_decorations {buff}
            };

        case component::map_id:
            return std::unique_ptr<structured_component_base> {
                new structured_component_map_id {buff}
            };

        case component::map_post_processing:
            return std::unique_ptr<structured_component_base> {
                new structured_component_map_post_processing {buff}
            };

        case component::max_damage:
            return std::unique_ptr<structured_component_base> {
                new structured_component_max_damage {buff}
            };

        case component::max_stack_size:
            return std::unique_ptr<structured_component_base> {
                new structured_component_max_stack_size {buff}
            };

        case component::minimum_attack_charge:
            return std::unique_ptr<structured_component_base> {
                new structured_component_minimum_attack_charge {buff}
            };

        case component::mooshroom_variant:
            return std::unique_ptr<structured_component_base> {
                new structured_component_mooshroom_variant {buff}
            };

        case component::note_block_sound:
            return std::unique_ptr<structured_component_base> {
                new structured_component_note_block_sound {buff}
            };

        case component::ominous_bottle_amplifier:
            return std::unique_ptr<structured_component_base> {
                new structured_component_ominous_bottle_amplifier {buff}
            };

        case component::painting_variant:
            return std::unique_ptr<structured_component_base> {
                new structured_component_painting_variant {buff}
            };

        case component::parrot_variant:
            return std::unique_ptr<structured_component_base> {
                new structured_component_parrot_variant {buff}
            };

        case component::piercing_weapon:
            return std::unique_ptr<structured_component_base> {
                new structured_component_piercing_weapon {buff}
            };

        case component::pig_sound_variant:
            return std::unique_ptr<structured_component_base> {
                new structured_component_pig_sound_variant {buff}
            };

        case component::pig_variant:
            return std::unique_ptr<structured_component_base> {
                new structured_component_pig_variant {buff}
            };

        case component::pot_decorations:
            return std::unique_ptr<structured_component_base> {
                new structured_component_pot_decorations {buff}
            };

        case component::potion_contents:
            return std::unique_ptr<structured_component_base> {
                new structured_component_potion_contents {buff}
            };

        case component::potion_duration_scale:
            return std::unique_ptr<structured_component_base> {
                new structured_component_potion_duration_scale {buff}
            };

        case component::profile:
            return std::unique_ptr<structured_component_base> {
                new structured_component_profile {buff}
            };

        case component::provides_banner_patterns:
            return std::unique_ptr<structured_component_base> {
                new structured_component_provides_banner_patterns {buff}
            };

        case component::provides_trim_material:
            return std::unique_ptr<structured_component_base> {
                new structured_component_provides_trim_material {buff}
            };

        case component::rabbit_variant:
            return std::unique_ptr<structured_component_base> {
                new structured_component_rabbit_variant {buff}
            };

        case component::rarity:
            return std::unique_ptr<structured_component_base> {
                new structured_component_rarity {buff}
            };

        case component::recipes:
            return std::unique_ptr<structured_component_base> {
                new structured_component_recipes {buff}
            };

        case component::repair_cost:
            return std::unique_ptr<structured_component_base> {
                new structured_component_repair_cost {buff}
            };

        case component::repairable:
            return std::unique_ptr<structured_component_base> {
                new structured_component_repairable {buff}
            };

        case component::salmon_size:
            return std::unique_ptr<structured_component_base> {
                new structured_component_salmon_size {buff}
            };

        case component::sheep_color:
            return std::unique_ptr<structured_component_base> {
                new structured_component_sheep_color {buff}
            };

        case component::shulker_color:
            return std::unique_ptr<structured_component_base> {
                new structured_component_shulker_color {buff}
            };

        case component::stored_enchantments:
            return std::unique_ptr<structured_component_base> {
                new structured_component_stored_enchantments {buff}
            };

        case component::sulfur_cube_content:
            return std::unique_ptr<structured_component_base> {
                new structured_component_sulfur_cube_content {buff}
            };

        case component::suspicious_stew_effects:
            return std::unique_ptr<structured_component_base> {
                new structured_component_suspicious_stew_effects {buff}
            };

        case component::swing_animation:
            return std::unique_ptr<structured_component_base> {
                new structured_component_swing_animation {buff}
            };

        case component::tool:
            return std::unique_ptr<structured_component_base> {
                new structured_component_tool {buff}
            };

        case component::tooltip_display:
            return std::unique_ptr<structured_component_base> {
                new structured_component_tooltip_display {buff}
            };

        case component::tooltip_style:
            return std::unique_ptr<structured_component_base> {
                new structured_component_tooltip_style {buff}
            };

        case component::trim:
            return std::unique_ptr<structured_component_base> {
                new structured_component_trim {buff}
            };

        case component::tropical_fish_base_color:
            return std::unique_ptr<structured_component_base> {
                new structured_component_tropical_fish_base_color {buff}
            };

        case component::tropical_fish_pattern:
            return std::unique_ptr<structured_component_base> {
                new structured_component_tropical_fish_pattern {buff}
            };

        case component::tropical_fish_pattern_color:
            return std::unique_ptr<structured_component_base> {
                new structured_component_tropical_fish_pattern_color {buff}
            };

        case component::unbreakable:
            return std::unique_ptr<structured_component_base> {
                new structured_component_unbreakable {buff}
            };

        case component::use_cooldown:
            return std::unique_ptr<structured_component_base> {
                new structured_component_use_cooldown {buff}
            };

        case component::use_effects:
            return std::unique_ptr<structured_component_base> {
                new structured_component_use_effects {buff}
            };

        case component::use_remainder:
            return std::unique_ptr<structured_component_base> {
                new structured_component_use_remainder {buff}
            };

        case component::villager_variant:
            return std::unique_ptr<structured_component_base> {
                new structured_component_villager_variant {buff}
            };

        case component::weapon:
            return std::unique_ptr<structured_component_base> {
                new structured_component_weapon {buff}
            };

        case component::wolf_collar:
            return std::unique_ptr<structured_component_base> {
                new structured_component_wolf_collar {buff}
            };

        case component::wolf_sound_variant:
            return std::unique_ptr<structured_component_base> {
                new structured_component_wolf_sound_variant {buff}
            };

        case component::wolf_variant:
            return std::unique_ptr<structured_component_base> {
                new structured_component_wolf_variant {buff}
            };

        case component::writable_book_content:
            return std::unique_ptr<structured_component_base> {
                new structured_component_writable_book_content {buff}
            };

        case component::written_book_content:
            return std::unique_ptr<structured_component_base> {
                new structured_component_written_book_content {buff}
            };

        case component::zombie_nautilus_variant:
            return std::unique_ptr<structured_component_base> {
                new structured_component_zombie_nautilus_variant {buff}
            };
    }
}
