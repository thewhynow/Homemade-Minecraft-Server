#pragma once

#include "inc/types.hpp"
#include "inc/connection.hpp"

class player {

public:
    int32_t entity_id;
    net_uuid id;
    std::string name;

    connection conn;

    net_position pos;
    net_position sent_pos;

    int32_t view_distance;
};
