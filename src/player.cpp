#include "inc/player.hpp"

player::player(
    net_game_profile &&profile,
    connection &conn
):
    entity(profile.uuid()),
    profile(std::move(profile)),
    conn(conn),
    teleport_id(0)
{}

void player::tick() {
    int32_t new_cx = (int32_t) floor(pos.x) >> 4,
            new_cz = (int32_t) floor(pos.z) >> 4
    ;

    if (new_cx != chunk_x || new_cz != chunk_z){
        set_center_chunk();
    }
}

void player::on_enter_world(world &w){
    dimension = &w;

    pos = {0.0, 60.0, 0.0};
    vel = {0.0, 0.0, 0.0};
    pitch = 0.0f;
    yaw = 0.0f;
    chunk_x = 0.0;
    chunk_z = 0.0;

    packet_player_position sync = {
        {(uint8_t) packet_id::play::player_position},
        {(int32_t) ++teleport_id},
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

void player::set_center_chunk(){
    
}

void player::recieve(const packet_accept_teleportation &packet) {
    if ((uint32_t) packet.id() != teleport_id)
        return;

    pos = {packet.x(), packet.y(), packet.z()};
    pitch = packet.pitch();
    yaw = packet.yaw();
}

void player::recieve(const packet_move_player_position_rotation &packet) {
    pos = {packet.x(), packet.feet_y() - 1.62, packet.z()};
    yaw = packet.yaw();
    pitch = packet.pitch();
    on_ground = packet.flags() &
        (uint8_t) packet_move_player_position_rotation::flags_bitfields::on_ground
    ;
}
