#include "inc/player.hpp"

player::player(
    net_game_profile &&profile,
    connection &conn
):
    entity(profile.uuid()),
    profile(std::move(profile)),
    conn(conn)
{}

void player::tick() {
    /* like what? */
}
