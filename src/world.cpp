#include "inc/world.hpp"
#include "inc/blocks.hpp"

world::world() = default;

world world::overworld;

uint32_t world::get_block(int32_t x, int32_t y, int32_t z) const {
    if (y < min_y || max_y <= y)
        return block::air;

    chunk &c = chunk_loader::instance.load_chunk(
        x >> 4, z >> 4
    );

    y -= min_y;

    return c.sections[y / 16]
            .blocks[y % 16][z & 15][x & 15]
    ;
}

void world::set_block(int32_t x, int32_t y, int32_t z, uint32_t state) {
    if (y < min_y || max_y <= y)
        return;

    chunk &c = chunk_loader::instance.load_chunk(
        x >> 4, z >> 4
    );

    y -= min_y;

    c.sections[y / 16].blocks[y % 16][z & 15][x & 15]
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

