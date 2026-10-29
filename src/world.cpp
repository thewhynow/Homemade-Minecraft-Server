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

player *world::add_player(player &&plr) {
    uint32_t id = plr.id;
    player *plr_p = new player {std::move(plr)};
    plr_p->on_enter_world(*this);
    entities[plr_p->id] = std::unique_ptr<entity> {(entity*) plr_p};
    return (player*) entities[id].get();
}

void world::rem_player(player *player) {
    auto kv_pair = entities.extract(player->id);
    to_destroy.insert(std::move(kv_pair));
}

void world::tick() {
    for (auto &[id, entity] : entities)
        entity->tick();

    to_destroy.clear();
}

