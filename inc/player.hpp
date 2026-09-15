#pragma once

#include "inc/types.hpp"
#include "inc/connection.hpp"
#include "inc/packet.hpp"

#include <unordered_set>

class player {
public:
    template <typename... Ps>
    requires (is_packet_v<Ps> && ...)
    void recieve(const std::variant<Ps...> &p){
        (
            [&]() -> bool {
                if (
                    std::holds_alternative<Ps>(p)
                ){
                    return true;
                }

                return false;
            }
            || ...
        );
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
private:
};
