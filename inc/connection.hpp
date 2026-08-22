#pragma once
#include "socket.hpp"
#include <vector>
#include <span>
#include "types.hpp"

class connection {
public:
    connection(socket_wrapper &&sock);
public:
    void on_read();
    void on_write();
    bool is_dead();
    bool has_outbound_data();
public:
    enum connection_state {
        dead,
        handshake,
        status_request,
        status_ping,
        login_start,
        login_success,
        configuration,
        configuration_finish,
        play
    };
private:
    template<typename T>
    void queue_packet(const T &packet);

    /* only used with an r-value */
    void handle(std::span<uint8_t> &&buff);

    void handle_configuration(
        std::span<uint8_t> &buff
    );

    void handle_play(std::span<uint8_t> &buff);
private:
    socket_wrapper sock;
    connection_state state;

    std::vector<uint8_t> outbound;
    std::vector<uint8_t> inbound_buff;
    net_var_int inbound_size;
};
