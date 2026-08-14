#include "inc/connection.hpp"
#include <sys/socket.h>
#include <unistd.h>
#include <span>
#include <cstdlib>
#include "inc/packet.hpp"

connection::connection(socket_wrapper &&sock):
    sock(std::move(sock)),
    state(handshake),
    inbound_size(0)
{}

void connection::on_read(){
    uint8_t temp_buff[4096];

    if (state == dead)
        return;

    /* know there is data to read so dw about throw */
    ssize_t recieved = sock.recv(
        temp_buff, sizeof temp_buff
    );

    if (recieved == 0){
        state = dead;
        return;
    }
    else if (recieved < 0)
        return;

    inbound_buff.insert(
        inbound_buff.end(), temp_buff, 
        temp_buff + recieved
    );

    while (true){
        std::span<uint8_t> s(inbound_buff);

        net_var_int size(0);
        try {
            size = net_var_int(s);
        }
        catch (const std::length_error&){ 
            return; /* waiting for more */
        }
        catch (const std::exception&){
            state = dead;
            return; /* malformed */
        }

        if (size.value < 0 || size.value > 2097151){
            state = dead;
            return; /* malformed */
        }

        if (s.size() < (size_t) size.value)
            return; /* packet incomplete */

        handle(
            std::span(
                inbound_buff.begin() + size.size(),
                size.value
            )
        );

        inbound_buff.erase(
            inbound_buff.begin(),
            inbound_buff.begin()
            + size.size() + size.value
        );
    }
}

void connection::on_write(){
    if (!outbound.size())
        return;

    size_t sent;
    try {
        sent = sock.send(
            outbound.data(), outbound.size()
        );
    } catch (const std::exception &){
        state = dead;
        return;
    }

    outbound.erase(
        outbound.begin(), outbound.begin() + sent
    );
}

bool connection::is_dead(){
    return state == dead;
}

void connection::handle(
    std::span<uint8_t> &&buff
){
    switch (state){
        case dead: {
            break;
        }

        case handshake: {
            packet_intention packet(buff);
            if (
                packet.intent.value 
                == packet_intention::intent_status
            )
                state = status_request;
            break;
        }

        case status_request: {
            packet_status_request packet(buff);

            packet_status_response response(
                "{                                  \n"
                "    \"version\": {                 \n"
                "        \"name\": \"1.21.8\",      \n"
                "        \"protocol\": 776          \n"
                "    },                             \n"
                "    \"description\": {             \n"
                "        \"text\": \"Hello, World!\"\n"
                "    },                             \n"
                "    \"enforcesSecureChat\": false  \n"
                "}                                  \n"
            );

            queue_packet(response);
            state = status_ping;
            break;
        }

        case status_ping: {
            packet_ping_request packet(buff);

            packet_pong_response response(
                packet.timestamp
            );

            queue_packet(response);
            state = dead;
            break;
        }

        case login: {

        }

        case configuration: {

        }

        case play: {

        }
    }
}

template <typename T>
void connection::queue_packet(
    const T &packet
){
    std::vector<uint8_t> serialized;
    packet.serialize(serialized);
    outbound.insert(
        outbound.end(), 
        serialized.begin(), 
        serialized.end()
    );
}
