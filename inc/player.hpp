#pragma once

#include "inc/types.hpp"
#include "inc/connection.hpp"
#include "inc/packet.hpp"
#include "inc/entity.hpp"

#include <unordered_set>

class world;

class player : public entity {
public:
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
    void on_enter_world(world &w);
private:
    void set_center_chunk(int32_t new_cx, int32_t new_cz);
public:
    net_game_profile profile;

    connection &conn;

    std::unordered_set<
        /* x, z */
        std::pair<int32_t, int32_t>
    > loaded_chunks;
private:
    std::optional<int32_t> teleport_id;

private:
    void recieve(const packet_accept_teleportation &packet);

    void recieve(const packet_move_player_position_rotation &packet);

    void recieve(const packet_player_loaded &packet);
};
