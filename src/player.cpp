#include "inc/player.hpp"
#include "inc/packet.hpp"
#include "inc/chunk.hpp"
#include "inc/world.hpp"

#include <print>

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

    time_since_sent_heartbeat += 0.05;

    if (time_since_sent_heartbeat < 10.0)
        return;

    if (heartbeat_id)
        on_leave();
    else {
        heartbeat_id = rand();

        packet_keep_alive heartbeat = {
            (uint8_t) packet_id::play::heartbeat_client_bound,
            *heartbeat_id
        };

        conn.queue_packet(heartbeat);

        time_since_sent_heartbeat = 0;
    }
}

void player::on_enter_world(world &w){
    dimension = &w;

    pos = {0.0, 1.0, 0.0};
    vel = {0.0, 0.0, 0.0};
    pitch = 0.0f;
    yaw = 0.0f;

    /* force set_center_chunk on first tick */
    chunk_x = INT32_MIN;
    chunk_z = INT32_MIN;

    teleport_id = std::rand();

    packet_player_position sync = {
        {*teleport_id},
        {pos.x}, {pos.y}, {pos.z},
        {vel.x}, {vel.y}, {vel.z},
        {yaw}, {pitch},
        {0}
    };
    conn.queue_packet(sync);

    packet_game_event wait_for_chunks {
        {(uint8_t) packet_game_event::events::wait_for_chunks},
        {0.0f}
    };
    conn.queue_packet(wait_for_chunks);
}

void player::set_center_chunk(int32_t new_cx, int32_t new_cz){
    packet_set_center_chunk center_chunk = {
        {new_cx}, {new_cz}
    };
    conn.queue_packet(center_chunk);

    /* send a 5x5 area surrounding the center chunk, fuck render distance */
    for (int32_t xi = new_cx - 2; xi <= new_cx + 2; ++xi)
        for (int32_t zi = new_cz - 2; zi <= new_cz + 2; ++zi){
            const chunk &c = chunk_loader::instance.load_chunk(new_cx, new_cz);
            std::vector<uint8_t> chunk_bytes;
            for (const chunk_section &s : c.sections)
                net_chunk_section{s}.serialize(chunk_bytes);

            packet_level_chunk_with_light chunk = {
                {xi}, {zi},
                {{/* don't send heightmaps */}},
                /* uint8_t -> net_ubyte */
                {{chunk_bytes.begin(), chunk_bytes.end()}},
                {{/* no block entities, for now */}},
                /* constructs to light data */
                net_light_data{c}
            };
            /* poopy */

            conn.queue_packet(chunk);
        }
}

void player::on_leave(){
    conn.state = connection::connection_state::dead;
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
    /* what am i supposed to do here? */
}

void player::recieve(const packet_keep_alive &packet) {
    if (packet.id() != heartbeat_id)
        on_leave();
    else
        heartbeat_id = std::nullopt;
}
