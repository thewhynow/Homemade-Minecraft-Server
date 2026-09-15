#pragma once
#include "types.hpp"

#include <variant>

/* PROTOCOL VERSION 776 */

namespace packet_id {
    enum handshake : uint8_t {
        intention = 0
    };

    enum status : uint8_t {
        response = 0,
        pong     = 1,
        request  = 0,
        ping     = 1
    };

    enum login : uint8_t {
        hello        = 0,
        finished     = 2,
        acknowledged = 3
    };

    enum configuration : uint8_t {
        known_client_bound  = 14,
        known_server_bound  = 7,
        registry            = 7,
        finish              = 3,
        custom_client_bound = 1,
        custom_server_bound = 2,
        client_information  = 0,
        update_tags         = 13,
    };

    enum play : uint8_t {
        login                  = 49,
        player_position        = 72,
        confirm_teleportation  = 0,
        mov_player_pos_rot     = 31,
        player_info_update     = 70,
        game_event             = 38,
        set_chunk_cache_center = 94,
        level_with_chunk_light = 45,
        player_loaded          = 44,
    };
};

template<uint8_t Id, typename... Ts>
requires (
    std::is_base_of_v<net_type, Ts> && ...
)
struct packet;

template <typename T>
struct is_packet : std::false_type {};

template <uint8_t Id, typename... Ts>
struct is_packet<packet<Id, Ts...>> : std::true_type {};

template <typename T>
inline constexpr bool is_packet_v = is_packet<T>::value;

template<uint8_t Id, typename... Ts>
requires (
    std::is_base_of_v<net_type, Ts> && ...
)
struct packet :
    net_compound<net_var_int, Ts...>
{
    using body = net_compound<net_var_int, Ts...>;
    using body::body;

    auto &id(){
        return this-> template get<0>();
    }

    const auto &id() const {
        return this-> template get<0>();
    }

    void serialize(std::vector<uint8_t> &buff) const {
        net_var_int(body::size()).serialize(buff);
        body::serialize(buff);
    }

    /* for net_compound compatbility */
    size_t size() const {
        size_t len = body::size();
        return net_var_int(len).size() + len;
    }

    template <typename... Ps>
    requires (is_packet_v<Ps> && ...)
    static std::variant<Ps...> generic (
        std::span<uint8_t> &buff
    ){
        uint8_t id = buff[0];
        std::variant<Ps...> res;

        (
            [&](){
                if (id == Ps::Id){
                    res = Ps{buff};
                    return true;
                }

                return false;
            }
            || ...
        );

        return res;

        /*
        i'm going to keep this code here as a reminder of what could've
        been if apple clang decided to update faster...

        template for (constexpr size_t i = 0; i < sizeof... Ps; ++i){
            using P = Ps...[i];

            if (id == P::Id)
                return P{buff};
        }
        */
    }
};

#define PACKET_FIELD(num, name)                                        \
    auto &name(){ return this-> template get<num + 1>(); }             \
    const auto &name() const { return this-> template get<num + 1>(); }


struct packet_intention :
    packet<
        packet_id::handshake::intention,
        net_var_int,
        net_string,
        net_ushort,
        net_var_int
    >
{
    using packet::packet;

    PACKET_FIELD(0, version);
    PACKET_FIELD(1, address);
    PACKET_FIELD(2, port);
    PACKET_FIELD(3, intent);

    enum intents {
        intent_status   = 1,
        intent_login    = 2,
        intent_transfer = 3
    };
};

struct packet_status_response :
    packet<
        packet_id::status::response,
        net_string
    >
{
    using packet::packet;

    PACKET_FIELD(0, json_response);
};

struct packet_pong_response :
    packet<
        packet_id::status::pong,
        net_long
    >
{
    using packet::packet;

    PACKET_FIELD(0, timestamp);
};

struct packet_status_request :
    packet<packet_id::status::request>
{
    using packet::packet;
};

struct packet_ping_request :
    packet<
        packet_id::status::ping,
        net_long
    >
{
    using packet::packet;

    PACKET_FIELD(0, timestamp);
};

struct packet_hello :
    packet<
        packet_id::login::hello,
        net_string,
        net_uuid
    >
{
    using packet::packet;

    PACKET_FIELD(0, name);
    PACKET_FIELD(1, player_uuid);
};

struct packet_login_finished :
    packet<
        packet_id::login::finished,
        net_game_profile,
        net_uuid
    >
{
    using packet::packet;

    PACKET_FIELD(0, profile);
    PACKET_FIELD(1, session_id);
};

struct packet_login_acknowledged :
    packet<packet_id::login::acknowledged>
{
    using packet::packet;
};

struct packet_select_known_packs :
    packet<
        (uint8_t) -1, /* differing based on server / client bound */
        net_prefixed_array<
            net_select_known_packs_known_pack
        >
    >
{
    using packet::packet;

    PACKET_FIELD(0, known_packs);
};

struct packet_registry_data :
    packet<
        packet_id::configuration::registry,
        net_identifier,
        net_prefixed_array<
            net_registry_data_entry
        >
    >
{
    using packet::packet;

    PACKET_FIELD(0, id);
    PACKET_FIELD(1, entries);
};

struct packet_finish_configuration :
    packet<packet_id::configuration::finish>
{
    using packet::packet;
};

struct packet_custom_payload_plugin_message:
    packet<
        (uint8_t) -1, /* differing based on server / client bound */
        net_identifier,
        net_string
    >
{
    using packet::packet;

    PACKET_FIELD(0, channel);
    PACKET_FIELD(1, brand);
};

struct packet_client_information :
    packet<
        packet_id::configuration::client_information,
        net_string,
        net_byte,
        net_var_int,
        net_boolean,
        net_ubyte,
        net_var_int,
        net_boolean,
        net_boolean,
        net_var_int
    >
{
    using packet::packet;

    PACKET_FIELD(0, locale);
    PACKET_FIELD(1, view_distance);
    PACKET_FIELD(2, chat_mode);
    PACKET_FIELD(3, chat_colors);
    PACKET_FIELD(4, skin_parts);
    PACKET_FIELD(5, hand);
    PACKET_FIELD(6, text_filtering);
    PACKET_FIELD(7, server_listings);
    PACKET_FIELD(8, particles);

    enum chat_modes {
        enabled = 0,
        commands = 1,
        hidden = 2
    };

    enum skin_parts_masks : uint8_t {
        cape         = 0x01,
        jacket       = 0x02,
        left_sleeve  = 0x04,
        right_sleeve = 0x08,
        left_pants   = 0x10,
        right_pants  = 0x20,
        hat          = 0x40
    };

    enum main_hand {
        left  = 0,
        right = 1
    };

    enum particle_statuses {
        all       = 0,
        decreased = 1,
        minimal   = 2
    };
};

struct packet_login :
    packet<
        packet_id::play::login,
        net_int,
        net_boolean,
        net_prefixed_array<
            net_identifier
        >,
        net_var_int,
        net_var_int,
        net_var_int,
        net_boolean,
        net_boolean,
        net_boolean,
        net_var_int,
        net_identifier,
        net_long,
        net_ubyte,
        net_byte,
        net_boolean,
        net_boolean,
        net_prefixed_optional<
            net_login_death_location
        >,
        net_var_int,
        net_var_int,
        net_boolean,
        net_boolean
    >
{
    using packet::packet;

    PACKET_FIELD(0,  entity_id);
    PACKET_FIELD(1,  hardcore);
    PACKET_FIELD(2,  dimension_names);
    PACKET_FIELD(3,  max_players);
    PACKET_FIELD(4,  view_distance);
    PACKET_FIELD(5,  simulation_distance);
    PACKET_FIELD(6,  reduced_debug_info);
    PACKET_FIELD(7,  enable_respawn_screen);
    PACKET_FIELD(8,  limited_crafting);
    PACKET_FIELD(9,  dimension_type);
    PACKET_FIELD(10, dimension_name);
    PACKET_FIELD(11, hashed_seed);
    PACKET_FIELD(12, game_mode);
    PACKET_FIELD(13, prev_game_mode);
    PACKET_FIELD(14, debug_world);
    PACKET_FIELD(15, flat_world);
    PACKET_FIELD(16, death_location);
    PACKET_FIELD(17, portal_cooldown);
    PACKET_FIELD(18, sea_level);
    PACKET_FIELD(19, online_mode);
    PACKET_FIELD(20, enforces_secure_chat);

    enum game_modes : uint8_t {
        survival  = 0,
        creative  = 1,
        adventure = 2,
        spectator = 3
    };
};

struct packet_update_tags :
    packet<
        packet_id::configuration::update_tags,
        net_prefixed_array<
            net_update_tags_tagged_registry
        >
    >
{
    using packet::packet;

    PACKET_FIELD(0, tagged_registries);
};

struct packet_level_chunk_with_light :
    packet<
        packet_id::play::level_with_chunk_light,
        net_int,
        net_int,
        net_prefixed_array<net_heightmap>,
        net_prefixed_array<net_byte>,
        net_prefixed_array<net_level_chunk_with_light_block_entity>,
        net_light_data
    >
{
    using packet::packet;

    PACKET_FIELD(0, chunk_x);
    PACKET_FIELD(1, chunk_z);
    PACKET_FIELD(2, heightmaps);
    PACKET_FIELD(3, data);
    PACKET_FIELD(4, block_entities);
    PACKET_FIELD(5, light);
};
