#pragma once

#include <unordered_map>

#include "inc/types.hpp"
#include "inc/chunk.hpp"
#include "inc/player.hpp"

class world {
private:
    world();
public:
    static world overworld;
public:
    uint32_t get_block(
        int32_t x, int32_t y, int32_t z
    ) const;

    void set_block(
        int32_t x, int32_t y, int32_t z, uint32_t state
    );

    /* returns a NON-OWNING POINTER */
    player *add_player(player &&player);

    void tick();
private:
    static constexpr int32_t min_y = -64;
    static constexpr int32_t max_y = 320;

    int32_t spawn_x, spawn_z, spawn_y;

    std::unordered_map<
        uint32_t, std::unique_ptr<entity>
    > entities;
};
