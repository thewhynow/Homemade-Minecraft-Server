#include "inc/world.hpp"

world::world() = default;

world world::overworld;

uint32_t world::get_block(int32_t x, int32_t y, int32_t z) const {
    chunk &c = chunk_loader::instance.load_chunk(
        x / 16, z / 16
    );

    x = std::abs(x);
    y += std::abs(min_y);
    z = std::abs(z);

    return c.sections[y / 16]
            .blocks[x % 16][z % 16][y % 16]
    ;
}

void world::set_block(int32_t x, int32_t y, int32_t z, uint32_t state) {
    chunk &c = chunk_loader::instance.load_chunk(
        x / 16, z / 16
    );

    x = std::abs(x);
    y += std::abs(min_y);
    z = std::abs(z);

    c.sections[y / 16].blocks[x % 16][z % 16][y % 16]
        = state
    ;
}

player *world::add_player(player &&player) {
    uint32_t id = player.id;
    class player *plr = new class player {std::move(player)};
    plr->on_enter_world(*this);
    entities[player.id] = std::unique_ptr<entity> {(entity*) plr};
    return (class player*) entities[id].get();
}

void world::tick() {
    for (auto &[id, entity] : entities)
        entity->tick();
}

