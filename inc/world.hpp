#pragma once

#include <unordered_map>

#include "inc/types.hpp"
#include "chunk.hpp"
#include "blocks.hpp"

class world {
public:

private:
    static constexpr int32_t min_y = -64;
    static constexpr int32_t max_y = 320;

    net_position spawn;

    std::unordered_map<int32_t, std::unique_ptr<int>> players;

};
