#pragma once

#include <unordered_map>

#include "inc/types.hpp"

class world {
public:

private:
    static constexpr int32_t min_y = -64;
    static constexpr int32_t max_y = 384;

    net_position spawn;

    std::unordered_map<int32_t, std::unique_ptr<int>> players;

};
