#pragma once

#include <unordered_map>

#include "inc/types.hpp"
#include "inc/chunk.hpp"
#include "inc/player.hpp"

template <size_t H>
class world {
private:
    using chunk = chunk<H>;
public:
    uint32_t get_block(int32_t x, int32_t y, int32_t z) const;
    void     set_block(int32_t x, int32_t y, int32_t z, uint32_t state);

    const chunk &get_chunk(int32_t cx, int32_t cz) const;
    void unload_chunk(int32_t cx, int32_t cz);

    void add_player(player *player);

private:
    static constexpr int32_t min_y = -64;
    static constexpr int32_t max_y = H + min_y;

    int32_t spawn_x, spawn_z, spawn_y;

    std::unordered_map<int32_t, std::unique_ptr<player>> players;
};
