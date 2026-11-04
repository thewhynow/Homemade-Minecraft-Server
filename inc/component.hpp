#pragma once

#include "types.hpp"

struct structured_component_base {
    virtual ~structured_component_base() = default;

    virtual void serialize (std::vector<uint8_t> &buff) const = 0;
    virtual size_t size() const = 0;

    static std::unique_ptr<structured_component_base>
        build_structured_component(
            net_var_int type, std::span<uint8_t> &buff
        )
    ;
};

template <typename... Ts>
struct structured_component_compound :
    structured_component_base,
    net_compound<Ts...>
{
    using net_compound<Ts...>::net_compound;

    void serialize (std::vector<uint8_t> &buff) const override {
        net_compound<Ts...>::serialize(buff);
    }

    size_t size() const override {
        return net_compound<Ts...>::size();
    }
};

struct structured_component_custom_data :
    structured_component_compound<
        net_nbt_data
    >
{
    using structured_component_compound::structured_component_compound;

    NET_COMPOUND_FIELD(0, data);
};

struct structured_component_max_stack_size :
    structured_component_compound<
        net_var_int
    >
{
    using structured_component_compound::structured_component_compound;

    NET_COMPOUND_FIELD(0, max_stack_size);
};

struct structured_component_max_damage :
    structured_component_compound<
        net_var_int
    >
{
    using structured_component_compound::structured_component_compound;

    NET_COMPOUND_FIELD(0, max_damage);
};

struct structured_component_damage :
    structured_component_compound<
        net_var_int
    >
{
    using structured_component_compound::structured_component_compound;

    NET_COMPOUND_FIELD(0, damage);
};

struct structured_component_unbreakable :
    structured_component_compound<>
{
    using structured_component_compound::structured_component_compound;
};

struct structured_component_custom_name :
    structured_component_compound<
        net_text_component
    >
{
    using structured_component_compound::structured_component_compound;

    NET_COMPOUND_FIELD(0, name);
};

using structured_component_item_name = structured_component_custom_name;

struct structured_component_item_model :
    structured_component_compound<
        net_identifier
    >
{
    using structured_component_compound::structured_component_compound;

    NET_COMPOUND_FIELD(0, model);
};

struct structured_component_lore :
    structured_component_compound<
        net_prefixed_array<net_text_component>
    >
{
    using structured_component_compound::structured_component_compound;

    NET_COMPOUND_FIELD(0, lines);
};

struct structured_component_rarity :
    structured_component_compound<
        net_var_int
    >
{
    using structured_component_compound::structured_component_compound;

    NET_COMPOUND_FIELD(0, rarity);

    enum class rarities : int {
        common   = 0,
        uncommon = 1,
        rare     = 2,
        epic     = 3
    };
};

struct structured_component_enchantments_enchantment :
    net_compound<
        net_var_int,
        net_var_int
    >
{
    using net_compound::net_compound;

    NET_COMPOUND_FIELD(0, type_id);
    NET_COMPOUND_FIELD(1, level);
};

struct structured_component_enchantments :
    structured_component_compound<
        net_prefixed_array<structured_component_enchantments_enchantment>
    >
{
    using structured_component_compound::structured_component_compound;

    NET_COMPOUND_FIELD(0, enchantments);
};

struct structured_component_can_place_on :
    structured_component_compound<
        net_prefixed_array<net_block_predicate>
    >
{
    using structured_component_compound::structured_component_compound;

    NET_COMPOUND_FIELD(0, block_predicates);
};

using structured_component_can_break = structured_component_can_place_on;

struct structured_component_attribute_modifiers_attribute_modifier :
    net_compound<
        net_var_int,
        net_identifier,
        net_double,
        net_var_int,
        net_var_int
    >
{
    using net_compound::net_compound;

    NET_COMPOUND_FIELD(0, attribute_id);
    NET_COMPOUND_FIELD(1, modifier_id);
    NET_COMPOUND_FIELD(2, value);
    NET_COMPOUND_FIELD(3, operation);
    NET_COMPOUND_FIELD(4, slot);

    enum class operations {
        add = 0,
        multiply_base = 1,
        multiply_total = 2
    };

    enum class slots {
        any = 0,
        main_hand = 1,
        off_hand = 2,
        hand = 3,
        feet = 4,
        legs = 5,
        chest = 6,
        head = 7,
        armor = 8,
        body = 9
    };
};

struct structured_component_attribute_modifiers :
    structured_component_compound<
        net_prefixed_array<structured_component_attribute_modifiers_attribute_modifier>
    >
{
    using structured_component_compound::structured_component_compound;

    NET_COMPOUND_FIELD(0, modifiers);
};

struct structured_component_custom_model_data :
    structured_component_compound<
        net_prefixed_array<net_float>,
        net_prefixed_array<net_boolean>,
        net_prefixed_array<net_string>,
        net_prefixed_array<net_int>
    >
{
    using structured_component_compound::structured_component_compound;

    NET_COMPOUND_FIELD(0, floats);
    NET_COMPOUND_FIELD(1, flags);
    NET_COMPOUND_FIELD(2, strings);
    NET_COMPOUND_FIELD(3, colors);
};

struct structured_component_tooltip_display :
    structured_component_compound<
        net_boolean,
        net_prefixed_array<net_var_int>
    >
{
    using structured_component_compound::structured_component_compound;

    NET_COMPOUND_FIELD(0, hide_tooltip);
    NET_COMPOUND_FIELD(1, hidden_components);
};

struct structured_component_repair_cost :
    structured_component_compound<
        net_var_int
    >
{
    using structured_component_compound::structured_component_compound;

    NET_COMPOUND_FIELD(0, cost);
};

struct structured_component_creative_slot_lock :
    structured_component_compound<>
{
    using structured_component_compound::structured_component_compound;
};

struct structured_component_enchantment_glint_override :
    structured_component_compound<
        net_boolean
    >
{
    using structured_component_compound::structured_component_compound;

    NET_COMPOUND_FIELD(0, has_glint);
};

struct structured_component_intangible_projectile :
    structured_component_compound<
        net_nbt_data
    >
{
    using structured_component_compound::structured_component_compound;

    NET_COMPOUND_FIELD(0, empty);
};

struct structured_component_food :
    structured_component_compound<
        net_var_int,
        net_float,
        net_boolean
    >
{
    using structured_component_compound::structured_component_compound;

    NET_COMPOUND_FIELD(0, nutrition);
    NET_COMPOUND_FIELD(1, saturation_modifier);
    NET_COMPOUND_FIELD(2, can_always_eat);
};

struct structured_component_consumable :
    structured_component_compound<
        net_float,
        net_var_int,
        net_id_or_x<net_sound_event>,
        net_boolean,
        net_prefixed_array<net_consume_effect>
    >
{
    using structured_component_compound::structured_component_compound;

    NET_COMPOUND_FIELD(0, consume_seconds);
    NET_COMPOUND_FIELD(1, animation);
    NET_COMPOUND_FIELD(2, sound);
    NET_COMPOUND_FIELD(3, has_consume_particles);
    NET_COMPOUND_FIELD(4, effects);

    enum class animations {
        none      = 0,
        eat       = 1,
        drink     = 2,
        block     = 3,
        bow       = 4,
        spear     = 5,
        crossbow  = 6,
        spyglass  = 7,
        toot_horn = 8,
        brush     = 9
    };
};

struct structured_component_use_remainder :
    structured_component_compound<
        net_boxed<net_slot>
    >
{
    using structured_component_compound::structured_component_compound;

    NET_COMPOUND_FIELD(0, remainder);
};

struct structured_component_use_cooldown :
    structured_component_compound<
        net_float,
        net_prefixed_optional<net_identifier>
    >
{
    using structured_component_compound::structured_component_compound;

    NET_COMPOUND_FIELD(0, seconds);
    NET_COMPOUND_FIELD(1, cooldown_group);
};

struct structured_component_use_effects :
    structured_component_compound<
        net_boolean,
        net_boolean,
        net_float
    >
{
    using structured_component_compound::structured_component_compound;

    NET_COMPOUND_FIELD(0, can_sprint);
    NET_COMPOUND_FIELD(1, interact_vibrations);
    NET_COMPOUND_FIELD(2, speed_multiplier);
};

struct structured_component_minimum_attack_charge :
    structured_component_compound<
        net_float
    >
{
    using structured_component_compound::structured_component_compound;

    NET_COMPOUND_FIELD(0, minimum_attack_charge);
};

struct structured_component_damage_type :
    structured_component_compound<
        net_var_int
    >
{
    using structured_component_compound::structured_component_compound;

    NET_COMPOUND_FIELD(0, damage_type);
};

struct structured_component_damage_resistant :
    structured_component_compound<
        net_id_set
    >
{
    using structured_component_compound::structured_component_compound;

    NET_COMPOUND_FIELD(0, types);
};

struct structured_component_attack_range :
    structured_component_compound<
        net_float,
        net_float,
        net_float,
        net_float,
        net_float,
        net_float
    >
{
    using structured_component_compound::structured_component_compound;

    NET_COMPOUND_FIELD(0, min_reach);
    NET_COMPOUND_FIELD(1, max_reach);
    NET_COMPOUND_FIELD(2, min_creative_reach);
    NET_COMPOUND_FIELD(3, max_creative_reach);
    NET_COMPOUND_FIELD(4, hitbox_margin);
    NET_COMPOUND_FIELD(5, mob_factor);
};

struct structured_component_tool_rule :
    net_compound<
        net_id_set,
        net_prefixed_optional<net_float>,
        net_prefixed_optional<net_boolean>
    >
{
    using net_compound::net_compound;

    NET_COMPOUND_FIELD(0, blocks);
    NET_COMPOUND_FIELD(1, speed);
    NET_COMPOUND_FIELD(2, correct_drop_for_blocks);
};

struct structured_component_tool :
    structured_component_compound<
        net_prefixed_array<structured_component_tool_rule>,
        net_float,
        net_var_int,
        net_boolean
    >
{
    using structured_component_compound::structured_component_compound;

    NET_COMPOUND_FIELD(0, rules);
    NET_COMPOUND_FIELD(1, default_mining_speed);
    NET_COMPOUND_FIELD(2, damage_per_block);
    NET_COMPOUND_FIELD(3, can_destroy_blocks_in_creative);
};

struct structured_component_weapon :
    structured_component_compound<
        net_var_int,
        net_float
    >
{
    using structured_component_compound::structured_component_compound;

    NET_COMPOUND_FIELD(0, damage_per_attack);
    NET_COMPOUND_FIELD(1, disable_blocking_for); /* in seconds */
};

struct structured_component_enchantable :
    structured_component_compound<
        net_var_int
    >
{
    using structured_component_compound::structured_component_compound;

    NET_COMPOUND_FIELD(0, value);
};

struct structured_component_equippable :
    structured_component_compound<
        net_var_int,
        net_id_or_x<net_sound_event>,
        net_prefixed_optional<net_identifier>,
        net_prefixed_optional<net_identifier>,
        net_prefixed_optional<net_id_set>,
        net_boolean,
        net_boolean,
        net_boolean,
        net_boolean,
        net_id_or_x<net_sound_event>
    >
{
    using structured_component_compound::structured_component_compound;

    NET_COMPOUND_FIELD(0, slot);
    NET_COMPOUND_FIELD(1, equip_sound);
    NET_COMPOUND_FIELD(2, model);
    NET_COMPOUND_FIELD(3, camera_overlay);
    NET_COMPOUND_FIELD(4, allowed_entities);
    NET_COMPOUND_FIELD(5, dispensable);
    NET_COMPOUND_FIELD(6, swappable);
    NET_COMPOUND_FIELD(7, damage_on_hurt);
    NET_COMPOUND_FIELD(8, can_be_sheared);
    NET_COMPOUND_FIELD(9, shearing_sound);

    enum class slots {
        main_hand = 0,
        feet      = 1,
        legs      = 2,
        chest     = 3,
        head      = 4,
        off_hand  = 5,
        body      = 6
    };
};

struct structured_component_repairable :
    structured_component_compound<
        net_id_set
    >
{
    using structured_component_compound::structured_component_compound;

    NET_COMPOUND_FIELD(0, items);
};

struct structured_component_glider :
    structured_component_compound<>
{
    using structured_component_compound::structured_component_compound;
};

struct structured_component_tooltip_style :
    structured_component_compound<
        net_identifier
    >
{
    using structured_component_compound::structured_component_compound;

    NET_COMPOUND_FIELD(0, style);
};

struct structured_component_death_protection :
    structured_component_compound<
        net_prefixed_array<net_consume_effect>
    >
{
    using structured_component_compound::structured_component_compound;

    NET_COMPOUND_FIELD(0, effects);
};

struct structured_component_blocks_attacks_damage_reduction :
    net_compound<
        net_float,
        net_prefixed_optional<net_id_set>,
        net_float,
        net_float
    >
{
    using net_compound::net_compound;

    NET_COMPOUND_FIELD(0, horizontal_blocking_angle);
    NET_COMPOUND_FIELD(1, type);
    NET_COMPOUND_FIELD(2, base);
    NET_COMPOUND_FIELD(3, factor);
};

struct structured_component_blocks_attacks :
    structured_component_compound<
        net_float,
        net_float,
        net_prefixed_array<structured_component_blocks_attacks_damage_reduction>,
        net_float,
        net_float,
        net_float,
        net_prefixed_optional<net_id_set>,
        net_prefixed_optional<net_id_or_x<net_sound_event>>,
        net_prefixed_optional<net_id_or_x<net_sound_event>>
    >
{
    using structured_component_compound::structured_component_compound;

    NET_COMPOUND_FIELD(0, block_delay_seconds);
    NET_COMPOUND_FIELD(1, disable_cooldown_scale);
    NET_COMPOUND_FIELD(2, damage_reductions);
    NET_COMPOUND_FIELD(3, item_damage_threshold);
    NET_COMPOUND_FIELD(4, item_damage_base);
    NET_COMPOUND_FIELD(5, item_damage_factor);
    NET_COMPOUND_FIELD(6, bypassed_by);
    NET_COMPOUND_FIELD(7, block_sound);
    NET_COMPOUND_FIELD(8, disable_sound);
};

/* same layout as minecraft:enchantments */
using structured_component_stored_enchantments = structured_component_enchantments;

struct structured_component_dyed_color :
    structured_component_compound<
        net_int
    >
{
    using structured_component_compound::structured_component_compound;

    NET_COMPOUND_FIELD(0, color);
};

using structured_component_map_color = structured_component_dyed_color;

struct structured_component_map_id :
    structured_component_compound<
        net_var_int
    >
{
    using structured_component_compound::structured_component_compound;

    NET_COMPOUND_FIELD(0, id);
};

struct structured_component_map_decorations :
    structured_component_compound<
        net_nbt_data
    >
{
    using structured_component_compound::structured_component_compound;

    NET_COMPOUND_FIELD(0, data);
};

struct structured_component_map_post_processing :
    structured_component_compound<
        net_var_int
    >
{
    using structured_component_compound::structured_component_compound;

    NET_COMPOUND_FIELD(0, type);

    enum class types {
        lock  = 0,
        scale = 1
    };
};

struct structured_component_charged_projectiles :
    structured_component_compound<
        net_prefixed_array<net_slot>
    >
{
    using structured_component_compound::structured_component_compound;

    NET_COMPOUND_FIELD(0, projectiles);
};

struct structured_component_bundle_contents :
    structured_component_compound<
        net_prefixed_array<net_slot>
    >
{
    using structured_component_compound::structured_component_compound;

    NET_COMPOUND_FIELD(0, items);
};

struct structured_component_potion_contents :
    structured_component_compound<
        net_prefixed_optional<net_var_int>,
        net_prefixed_optional<net_int>,
        net_prefixed_array<net_potion_effect>,
        net_prefixed_optional<net_string>
    >
{
    using structured_component_compound::structured_component_compound;

    NET_COMPOUND_FIELD(0, potion_id);
    NET_COMPOUND_FIELD(1, custom_color);
    NET_COMPOUND_FIELD(2, custom_effects);
    NET_COMPOUND_FIELD(3, custom_name);
};

struct structured_component_potion_duration_scale :
    structured_component_compound<
        net_float
    >
{
    using structured_component_compound::structured_component_compound;

    NET_COMPOUND_FIELD(0, effect_multiplier);
};

struct structured_component_suspicious_stew_effects_effect :
    net_compound<
        net_var_int,
        net_var_int
    >
{
    using net_compound::net_compound;

    NET_COMPOUND_FIELD(0, type_id);
    NET_COMPOUND_FIELD(1, duration);
};

struct structured_component_suspicious_stew_effects :
    structured_component_compound<
        net_prefixed_array<structured_component_suspicious_stew_effects_effect>
    >
{
    using structured_component_compound::structured_component_compound;

    NET_COMPOUND_FIELD(0, effects);
};

struct structured_component_writable_book_content_page :
    net_compound<
        net_string,
        net_prefixed_optional<net_string>
    >
{
    using net_compound::net_compound;

    NET_COMPOUND_FIELD(0, raw_content);
    NET_COMPOUND_FIELD(1, filtered_content);
};

struct structured_component_writable_book_content :
    structured_component_compound<
        net_prefixed_array<structured_component_writable_book_content_page>
    >
{
    using structured_component_compound::structured_component_compound;

    NET_COMPOUND_FIELD(0, pages);
};

struct structured_component_written_book_content_page :
    net_compound<
        net_text_component,
        net_prefixed_optional<net_text_component>
    >
{
    using net_compound::net_compound;

    NET_COMPOUND_FIELD(0, raw_content);
    NET_COMPOUND_FIELD(1, filtered_content);
};

struct structured_component_written_book_content :
    structured_component_compound<
        net_string,
        net_prefixed_optional<net_string>,
        net_string,
        net_var_int,
        net_prefixed_array<structured_component_written_book_content_page>,
        net_boolean
    >
{
    using structured_component_compound::structured_component_compound;

    NET_COMPOUND_FIELD(0, raw_title);
    NET_COMPOUND_FIELD(1, filtered_title);
    NET_COMPOUND_FIELD(2, author);
    NET_COMPOUND_FIELD(3, generation);
    NET_COMPOUND_FIELD(4, pages);
    NET_COMPOUND_FIELD(5, resolved);
};

struct structured_component_trim :
    structured_component_compound<
        net_id_or_x<net_trim_material>,
        net_id_or_x<net_trim_pattern>
    >
{
    using structured_component_compound::structured_component_compound;

    NET_COMPOUND_FIELD(0, trim_material);
    NET_COMPOUND_FIELD(1, trim_pattern);
};

struct structured_component_debug_stick_state :
    structured_component_compound<
        net_nbt_data
    >
{
    using structured_component_compound::structured_component_compound;

    NET_COMPOUND_FIELD(0, data);
};

struct structured_component_entity_data :
    structured_component_compound<
        net_var_int,
        net_nbt_data
    >
{
    using structured_component_compound::structured_component_compound;

    NET_COMPOUND_FIELD(0, entity_type);
    NET_COMPOUND_FIELD(1, data);
};

struct structured_component_bucket_entity_data :
    structured_component_compound<
        net_nbt_data
    >
{
    using structured_component_compound::structured_component_compound;

    NET_COMPOUND_FIELD(0, data);
};

struct structured_component_block_entity_data :
    structured_component_compound<
        net_var_int,
        net_nbt_data
    >
{
    using structured_component_compound::structured_component_compound;

    NET_COMPOUND_FIELD(0, block_entity_type);
    NET_COMPOUND_FIELD(1, data);
};

struct structured_component_instrument :
    structured_component_compound<
        net_id_or_x<net_instrument>
    >
{
    using structured_component_compound::structured_component_compound;

    NET_COMPOUND_FIELD(0, instrument);
};

struct structured_component_piercing_weapon :
    structured_component_compound<
        net_boolean,
        net_boolean,
        net_prefixed_optional<net_sound_event>,
        net_prefixed_optional<net_sound_event>
    >
{
    using structured_component_compound::structured_component_compound;

    NET_COMPOUND_FIELD(0, deals_knockback);
    NET_COMPOUND_FIELD(1, dismounts);
    NET_COMPOUND_FIELD(2, sound);
    NET_COMPOUND_FIELD(3, hit_sound);
};

struct structured_component_kinetic_weapon :
    structured_component_compound<
        net_var_int,
        net_var_int,
        net_prefixed_optional<net_kinetic_weapon_conditions>,
        net_prefixed_optional<net_kinetic_weapon_conditions>,
        net_prefixed_optional<net_kinetic_weapon_conditions>,
        net_float,
        net_float,
        net_prefixed_optional<net_sound_event>,
        net_prefixed_optional<net_sound_event>
    >
{
    using structured_component_compound::structured_component_compound;

    NET_COMPOUND_FIELD(0, contact_cooldown_ticks);
    NET_COMPOUND_FIELD(1, delay_ticks);
    NET_COMPOUND_FIELD(2, dismount_conditions);
    NET_COMPOUND_FIELD(3, knockback_conditions);
    NET_COMPOUND_FIELD(4, damage_conditions);
    NET_COMPOUND_FIELD(5, forward_movement);
    NET_COMPOUND_FIELD(6, damage_multiplier);
    NET_COMPOUND_FIELD(7, sound);
    NET_COMPOUND_FIELD(8, hit_sound);
};

struct structured_component_swing_animation :
    structured_component_compound<
        net_var_int,
        net_var_int
    >
{
    using structured_component_compound::structured_component_compound;

    NET_COMPOUND_FIELD(0, type);
    NET_COMPOUND_FIELD(1, duration);

    enum class types {
        none  = 0,
        whack = 1,
        stab  = 2
    };
};

struct structured_component_additional_trade_cost :
    structured_component_compound<
        net_var_int
    >
{
    using structured_component_compound::structured_component_compound;

    NET_COMPOUND_FIELD(0, additional_trade_cost);
};

struct structured_component_dye :
    structured_component_compound<
        net_dye_color
    >
{
    using structured_component_compound::structured_component_compound;

    NET_COMPOUND_FIELD(0, color);
};

struct structured_component_provides_trim_material :
    structured_component_compound<
        net_id_or_x<net_trim_material>
    >
{
    using structured_component_compound::structured_component_compound;

    NET_COMPOUND_FIELD(0, material);
};

struct structured_component_ominous_bottle_amplifier :
    structured_component_compound<
        net_var_int
    >
{
    using structured_component_compound::structured_component_compound;

    NET_COMPOUND_FIELD(0, amplifier);
};

struct structured_component_jukebox_playable :
    structured_component_compound<
        net_id_or_x<net_jukebox_song>
    >
{
    using structured_component_compound::structured_component_compound;

    NET_COMPOUND_FIELD(0, jukebox_song);
};

struct structured_component_provides_banner_patterns :
    structured_component_compound<
        net_id_set
    >
{
    using structured_component_compound::structured_component_compound;

    NET_COMPOUND_FIELD(0, key);
};

struct structured_component_recipes :
    structured_component_compound<
        net_nbt_any
    >
{
    using structured_component_compound::structured_component_compound;

    NET_COMPOUND_FIELD(0, data);
};

struct structured_component_lodestone_tracker :
    structured_component_base,
    net_type
{
    net_boolean has_global_position;
    std::optional<net_identifier> dimension;
    std::optional<net_position> position;
    net_boolean tracked;

    structured_component_lodestone_tracker(std::span<uint8_t> &buff);

    void serialize(std::vector<uint8_t> &buff) const override;

    size_t size() const override;
};

struct structured_component_firework_explosion :
    structured_component_compound<
        net_firework_explosion
    >
{
    using structured_component_compound::structured_component_compound;

    NET_COMPOUND_FIELD(0, explosion);
};

struct structured_component_fireworks :
    structured_component_compound<
        net_var_int,
        net_prefixed_array<net_firework_explosion>
    >
{
    using structured_component_compound::structured_component_compound;

    NET_COMPOUND_FIELD(0, flight_duration);
    NET_COMPOUND_FIELD(1, explosions);
};

struct structured_component_profile :
    structured_component_compound<
        net_resolvable_profile
    >
{
    using structured_component_compound::structured_component_compound;

    NET_COMPOUND_FIELD(0, profile);
};

struct structured_component_note_block_sound :
    structured_component_compound<
        net_identifier
    >
{
    using structured_component_compound::structured_component_compound;

    NET_COMPOUND_FIELD(0, sound);
};

struct structured_component_banner_patterns_layer :
    net_compound<
        net_id_or_x<net_banner_pattern>,
        net_dye_color
    >
{
    using net_compound::net_compound;

    NET_COMPOUND_FIELD(0, pattern_type);
    NET_COMPOUND_FIELD(1, color);
};

struct structured_component_banner_patterns :
    structured_component_compound<
        net_prefixed_array<structured_component_banner_patterns_layer>
    >
{
    using structured_component_compound::structured_component_compound;

    NET_COMPOUND_FIELD(0, layers);
};

struct structured_component_base_color :
    structured_component_compound<
        net_dye_color
    >
{
    using structured_component_compound::structured_component_compound;

    NET_COMPOUND_FIELD(0, color);
};

struct structured_component_pot_decorations :
    structured_component_compound<
        net_prefixed_array<net_var_int>
    >
{
    using structured_component_compound::structured_component_compound;

    NET_COMPOUND_FIELD(0, decorations);
};

struct structured_component_container :
    structured_component_compound<
        net_prefixed_array<net_slot>
    >
{
    using structured_component_compound::structured_component_compound;

    NET_COMPOUND_FIELD(0, items);
};

struct structured_component_block_state_property :
    net_compound<
        net_string,
        net_string
    >
{
    using net_compound::net_compound;

    NET_COMPOUND_FIELD(0, name);
    NET_COMPOUND_FIELD(1, value);
};

struct structured_component_block_state :
    structured_component_compound<
        net_prefixed_array<structured_component_block_state_property>
    >
{
    using structured_component_compound::structured_component_compound;

    NET_COMPOUND_FIELD(0, properties);
};

struct structured_component_bees_bee :
    net_compound<
        net_var_int,
        net_nbt_data,
        net_var_int,
        net_var_int
    >
{
    using net_compound::net_compound;

    NET_COMPOUND_FIELD(0, entity_type);
    NET_COMPOUND_FIELD(1, entity_data);
    NET_COMPOUND_FIELD(2, ticks_in_hive);
    NET_COMPOUND_FIELD(3, min_ticks_in_hive);
};

struct structured_component_bees :
    structured_component_compound<
        net_prefixed_array<structured_component_bees_bee>
    >
{
    using structured_component_compound::structured_component_compound;

    NET_COMPOUND_FIELD(0, bees);
};

struct structured_component_lock :
    structured_component_compound<
        net_nbt_any
    >
{
    using structured_component_compound::structured_component_compound;

    NET_COMPOUND_FIELD(0, key);
};

struct structured_component_container_loot :
    structured_component_compound<
        net_nbt_data
    >
{
    using structured_component_compound::structured_component_compound;

    NET_COMPOUND_FIELD(0, data);
};

struct structured_component_break_sound :
    structured_component_compound<
        net_id_or_x<net_sound_event>
    >
{
    using structured_component_compound::structured_component_compound;

    NET_COMPOUND_FIELD(0, sound_event);
};

struct structured_component_sulfur_cube_content :
    structured_component_compound<
        net_boxed<net_slot>
    >
{
    using structured_component_compound::structured_component_compound;

    NET_COMPOUND_FIELD(0, content);
};

struct structured_component_variant_base :
    structured_component_compound<
        net_var_int
    >
{
    using structured_component_compound::structured_component_compound;

    NET_COMPOUND_FIELD(0, variant);
};

using structured_component_villager_variant = structured_component_variant_base;

using structured_component_wolf_variant = structured_component_variant_base;

using structured_component_wolf_sound_variant = structured_component_variant_base;

struct structured_component_wolf_collar :
    structured_component_compound<
        net_dye_color
    >
{
    using structured_component_compound::structured_component_compound;

    NET_COMPOUND_FIELD(0, color);
};

struct structured_component_fox_variant :
    structured_component_variant_base
{
    using structured_component_variant_base::structured_component_variant_base;

    enum class variants {
        red  = 0,
        snow = 1
    };
};

struct structured_component_salmon_size :
    structured_component_compound<
        net_var_int
    >
{
    using structured_component_compound::structured_component_compound;

    NET_COMPOUND_FIELD(0, type);

    enum class types {
        small  = 0,
        medium = 1,
        large  = 2
    };
};

struct structured_component_parrot_variant :
    structured_component_variant_base
{
    using structured_component_variant_base::structured_component_variant_base;

    enum class variants {
        red_blue    = 0,
        blue        = 1,
        green       = 2,
        yellow_blue = 3,
        gray        = 4
    };
};

struct structured_component_tropical_fish_pattern :
    structured_component_compound<
        net_var_int
    >
{
    using structured_component_compound::structured_component_compound;

    NET_COMPOUND_FIELD(0, pattern);

    enum class patterns {
        kob       = 0,
        sunstreak = 1,
        snooper   = 2,
        dasher    = 3,
        brinely   = 4,
        spotty    = 5,
        flopper   = 6,
        stripey   = 7,
        glitter   = 8,
        blockfish = 9,
        betty     = 10,
        clayfish  = 11
    };
};

struct structured_component_tropical_fish_base_color :
    structured_component_compound<
        net_dye_color
    >
{
    using structured_component_compound::structured_component_compound;

    NET_COMPOUND_FIELD(0, color);
};

struct structured_component_tropical_fish_pattern_color :
    structured_component_compound<
        net_dye_color
    >
{
    using structured_component_compound::structured_component_compound;

    NET_COMPOUND_FIELD(0, color);
};

struct structured_component_mooshroom_variant :
    structured_component_variant_base
{
    using structured_component_variant_base::structured_component_variant_base;

    enum class variants {
        red   = 0,
        brown = 1
    };
};

struct structured_component_rabbit_variant :
    structured_component_variant_base
{
    using structured_component_variant_base::structured_component_variant_base;

    enum class variants {
        brown           = 0,
        white           = 1,
        black           = 2,
        white_splotched = 3,
        gold            = 4,
        salt            = 5,
        evil            = 6
    };
};

using structured_component_pig_variant = structured_component_variant_base;

using structured_component_pig_sound_variant = structured_component_variant_base;

using structured_component_cow_variant = structured_component_variant_base;

using structured_component_cow_sound_variant = structured_component_variant_base;

using structured_component_chicken_variant = structured_component_variant_base;

using structured_component_chicken_sound_variant = structured_component_variant_base;

using structured_component_frog_variant = structured_component_variant_base;

struct structured_component_horse_variant :
    structured_component_variant_base
{
    using structured_component_variant_base::structured_component_variant_base;

    enum class variants {
        white      = 0,
        creamy     = 1,
        chestnut   = 2,
        brown      = 3,
        black      = 4,
        gray       = 5,
        dark_brown = 6
    };
};

struct structured_component_painting_variant :
    structured_component_compound<
        net_id_or_x<net_painting_variant>
    >
{
    using structured_component_compound::structured_component_compound;

    NET_COMPOUND_FIELD(0, variant);
};

struct structured_component_llama_variant :
    structured_component_variant_base
{
    using structured_component_variant_base::structured_component_variant_base;

    enum class variants {
        creamy = 0,
        white  = 1,
        brown  = 2,
        gray   = 3
    };
};

struct structured_component_axolotl_variant :
    structured_component_variant_base
{
    using structured_component_variant_base::structured_component_variant_base;

    enum class variants {
        lucy = 0,
        wild = 1,
        gold = 2,
        cyan = 3,
        blue = 4
    };
};

using structured_component_zombie_nautilus_variant = structured_component_variant_base;

using structured_component_cat_variant = structured_component_variant_base;

using structured_component_cat_sound_variant = structured_component_variant_base;

struct structured_component_cat_collar :
    structured_component_compound<
        net_dye_color
    >
{
    using structured_component_compound::structured_component_compound;

    NET_COMPOUND_FIELD(0, color);
};

struct structured_component_sheep_color :
    structured_component_compound<
        net_dye_color
    >
{
    using structured_component_compound::structured_component_compound;

    NET_COMPOUND_FIELD(0, color);
};

struct structured_component_shulker_color :
    structured_component_compound<
        net_dye_color
    >
{
    using structured_component_compound::structured_component_compound;

    NET_COMPOUND_FIELD(0, color);
};
