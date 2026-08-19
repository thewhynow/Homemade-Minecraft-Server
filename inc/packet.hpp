#pragma once
#include "types.hpp"

/* PROTOCOL VERSION 776 */

namespace packet_id {
    enum class handshake : uint8_t {
        intention = 0
    };

    enum class status : uint8_t {
        response = 0,
        pong     = 1,
        request  = 0,
        ping     = 1
    };

    enum class login : uint8_t {
        hello        = 0,
        finished     = 2,
        acknowledged = 3
    };

    enum class configuration : uint8_t {
        known_client_bound = 7,
        known_server_bound = 14,
        registry           = 7,
        finish             = 3
    };
};

template<typename... Ts>
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
};

#define PACKET_FIELD(num, name)                                        \
    auto &name(){ return this-> template get<num + 1>(); }             \
    const auto &name() const { return this-> template get<num + 1>(); }


struct packet_intention :
    packet<
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
    packet<net_string>
{
    using packet::packet;

    PACKET_FIELD(0, json_response);
};

struct packet_pong_response :
    packet<net_long>
{
    using packet::packet;

    PACKET_FIELD(0, timestamp);
};

struct packet_status_request :
    packet<>
{
    using packet::packet;
};

struct packet_ping_request :
    packet<net_long>
{
    using packet::packet;

    PACKET_FIELD(0, timestamp);
};

struct packet_hello :
    packet<net_string, net_uuid>
{
    using packet::packet;

    PACKET_FIELD(0, name);
    PACKET_FIELD(1, player_uuid);
};

struct packet_login_finished :
    packet<net_game_profile, net_uuid>
{
    using packet::packet;

    PACKET_FIELD(0, profile);
    PACKET_FIELD(1, session_id);
};

struct packet_login_acknowledged :
    packet<>
{
    using packet::packet;
};

struct packet_select_known_packs :
    packet<
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
