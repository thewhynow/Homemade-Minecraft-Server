#pragma once

#include <array>

struct inventory_slot {
    int item_id, count;
};

struct inventory {
    std::array<inventory_slot, 46> slots;

    enum class slots_array_index {
        craft_out = 0, /* 1 slot */
        craft_in  = 1,  /* 4 slots */
        armor     = 5,  /* 4 slots */
        main      = 9,  /* 27 slots */
        hotbar    = 36, /* 9 slots */
        offhand   = 45  /* 1 slot */
    };

    int selected_hotbar; /* 0 - 8 */
};
