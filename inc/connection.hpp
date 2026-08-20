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
        status_response,
        status_ping,
        login_start,
        login_success,
        configuration_select,
        configuration_registry,
        configuration_finish,
        play
    };
private:
    void handle(std::span<uint8_t> &&buff);

    template<typename T>
    void queue_packet(const T &packet);

    void queue_registry(
        std::string_view name, 
        std::vector<net_registry_data_entry> 
            &&entries
    );

private:
    socket_wrapper sock;
    connection_state state;

    std::vector<uint8_t> outbound;
    std::vector<uint8_t> inbound_buff;
    net_var_int inbound_size;
};
