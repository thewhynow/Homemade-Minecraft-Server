#pragma once
#include "types.hpp"

/* PROTOCOL VERSION 776 */

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

struct packet_registry_data : 
    packet<
        net_identifier,
        
