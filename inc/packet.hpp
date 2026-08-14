#pragma once
#include "types.hpp"
#include <span>

/* PROTOCOL VERSION 776 */

struct packet {
    net_var_int id;

    packet(std::span<uint8_t> &buff);
    packet(net_var_int id);
};

struct packet_intention : packet {
    net_var_int version;
    net_string  address; /* n = 255 */
    net_ushort  port;
    net_var_int intent;

    enum intents {
        intent_status   = 1,
        intent_login    = 2,
        intent_transfer = 3
    };

    packet_intention(std::span<uint8_t> &buff);
};

struct packet_status_response : packet {
    net_string  json_response; /* n = 32767 */

    packet_status_response(std::string_view json);

    void serialize(std::vector<uint8_t> &buff) const;
};

struct packet_pong_response : packet {
    net_long timestamp; /* match the one sent by client */

    packet_pong_response(net_long timestamp);

    void serialize(std::vector<uint8_t> &buff) const;
};

struct packet_status_request : packet {
    packet_status_request(std::span<uint8_t> &buff);
};

struct packet_ping_request : packet {
    net_long timestamp; /* may be any number */

    packet_ping_request(std::span<uint8_t> &buff);
};

struct packet_hello : packet {
    net_string username; /* n = 16 */

};
