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
public:
    enum connection_state {
        dead,
        handshake,
        status_request,
        status_response,
        status_ping,
        login,
        configuration,
        play
    };
private:
    void handle(std::span<uint8_t> &&buff);

    template<typename T>
    void queue_packet(const T &packet);
private:
    socket_wrapper sock;
    connection_state state;

    std::vector<uint8_t> outbound;
    std::vector<uint8_t> inbound_buff;
    net_var_int inbound_size;
};
