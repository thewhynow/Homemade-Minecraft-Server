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
    uint32_t entity_id;
    net_game_profile profile;

    connection &conn;

    double x, y, z;
    float yaw, pitch;
    bool on_ground;

    int32_t chunk_x, chunk_z;
    std::unordered_set<
        /* x, z */
        std::pair<int32_t, int32_t>
    > loaded_chunks;

};
