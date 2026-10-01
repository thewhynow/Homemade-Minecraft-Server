#pragma once

#include "inc/types.hpp"
#include "inc/connection.hpp"
#include "inc/packet.hpp"
#include "inc/entity.hpp"

#include <unordered_set>

class world;

class player : public entity {
public:
    using entity::entity;

    player (
        net_game_profile &&profile,
        connection &conn
    );

    template <typename... Ps>
    requires (is_packet_v<Ps> && ...)
    void recieve(const std::variant<Ps...> &p){
        (
            [&]() -> bool {
                if (
                    std::holds_alternative<Ps>(p)
                ){
                    recieve(std::get<Ps>(p));
                    return true;
                }

                return false;
            }()
            || ...
        );
    }

    void tick() override;
public:
    net_game_profile profile;

    connection &conn;

    std::unordered_set<
        /* x, z */
        std::pair<int32_t, int32_t>
    > loaded_chunks;
private:
    void recieve(const packet_level_chunk_with_light &packet);

    void recieve(const packet_login &packet);
};
