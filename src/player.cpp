#include "inc/player.hpp"
#include "inc/packet.hpp"
#include "inc/chunk.hpp"

player::player(
    net_game_profile &&profile,
    connection &conn
):
    entity(profile.uuid()),
    profile(std::move(profile)),
    conn(conn)
{}

void player::tick() {
    int32_t new_cx = (int32_t) floor(pos.x) >> 4,
            new_cz = (int32_t) floor(pos.z) >> 4
    ;

    if (new_cx != chunk_x || new_cz != chunk_z){
        set_center_chunk(new_cx, new_cz);

        chunk_x = new_cx;
        chunk_z = new_cz;
    }
}

void player::on_enter_world(world &w){
    dimension = &w;

    pos = {0.0, 60.0, 0.0};
    vel = {0.0, 0.0, 0.0};
    pitch = 0.0f;
    yaw = 0.0f;

    /* force set_center_chunk on first tick */

    chunk_x = INT32_MIN;
    chunk_z = INT32_MIN;

    teleport_id = std::rand();

    packet_player_position sync = {
        {(uint8_t) packet_id::play::player_position},
        {*teleport_id},
        {pos.x}, {pos.y}, {pos.z},
        {vel.x}, {vel.y}, {vel.z},
        {yaw}, {pitch},
        {0}
    };
    conn.queue_packet(sync);

    packet_game_event wait_for_chunks {
        {(uint8_t) packet_id::play::game_event},
        {(uint8_t) packet_game_event::events::wait_for_chunks},
        {0.0f}
    };
    conn.queue_packet(wait_for_chunks);
}

void player::set_center_chunk(int32_t new_cx, int32_t new_cz){
    packet_set_center_chunk center_chunk = {
        {(uint8_t) packet_id::play::set_chunk_cache_center},
        {new_cx}, {new_cz}
    };
    conn.queue_packet(center_chunk);

    const chunk &c = chunk_loader::instance.load_chunk(new_cx, new_cz);
    std::vector<uint8_t> chunk_bytes;
    for (const chunk_section &s : c.sections)
        net_chunk_section{s}.serialize(chunk_bytes);

    packet_level_chunk_with_light chunk = {
        {(uint8_t) packet_id::play::level_with_chunk_light},
        {new_cx}, {new_cz},
        {{/* don't send heightmaps */}},
        /* uint8_t -> net_ubyte */
        {{chunk_bytes.begin(), chunk_bytes.end()}},
        {{/* no block entities, for now */}},
        /* constructs to light data */
        net_light_data{c}
    };

    conn.queue_packet(chunk);
}

void player::recieve(const packet_accept_teleportation &packet) {
    /* pos was already set when the teleport was sent */
    if ((uint32_t) packet.id() != teleport_id)
        return;

    teleport_id = std::nullopt;
}

void player::recieve(const packet_move_player_position_rotation &packet) {
    pos = {packet.x(), packet.feet_y() - 1.62, packet.z()};
    yaw = packet.yaw();
    pitch = packet.pitch();
    on_ground = packet.flags() &
        (uint8_t) packet_move_player_position_rotation::flags_bitfields::on_ground
    ;
}

void player::recieve(const packet_player_loaded &packet) {

}
