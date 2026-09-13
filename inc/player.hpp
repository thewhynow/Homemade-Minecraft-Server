#pragma once

#include "inc/types.hpp"
#include "inc/connection.hpp"
#include "inc/packet.hpp"

#include <unordered_set>

class player {
public:
    template <typename P>
    requires (std::__is_specialization_v<P, packet>)
    void send(const P &p){
        conn.queue_packet(p);
    }

    void tick();
public:
    int32_t     entity_id;
    net_uuid    id;
    std::string name;
    net_game_profile_property properties;

    connection &conn;

    double x, y, z;
    float yaw, pitch;
    bool on_ground;

    int32_t chunk_x, chunk_z;
    std::unordered_set<
        /* x, z */
        std::pair<int32_t, int32_t>
    > loaded_chunks;

    packet_client_information info;

    net_position pos;
    net_position sent_pos;

    int32_t view_distance;
};
