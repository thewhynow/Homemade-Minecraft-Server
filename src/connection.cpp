#include "inc/connection.hpp"
#include <sys/socket.h>
#include <unistd.h>
#include <span>
#include <cstdlib>
#include "inc/packet.hpp"
#include "inc/errors.hpp"

connection::connection(socket_wrapper &&sock):
    sock(std::move(sock)),
    state(handshake),
    inbound_size(0)
{}

void connection::on_read(){
    uint8_t temp_buff[4096];

    if (state == dead)
        return;

    ssize_t recieved;
    try {
        recieved = sock.recv(
            temp_buff, sizeof temp_buff
        );
    }
    catch (const failed_read &){
        state = dead;
        return;
    }

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
        size_t before = s.size();
        net_var_int size(0);
        try {
            size = net_var_int(s);
        }
        catch (const unfinished_packet&){ 
            return;
        }
        catch (const malformed_packet&){
            state = dead;
            return;
        }

        size_t header = before - s.size();

        if (size < 0 || size > 2097151){
            state = dead;
            return;/* malformed */
        }

        if (s.size() < (size_t) size)
            return; /* packet incomplete */

        try {
            handle(s.subspan(0, size));
        }
        catch (const std::runtime_error &){
            state = dead;
            return;
        }

        inbound_buff.erase(
            inbound_buff.begin(),
            inbound_buff.begin() + header + size
        );
    }
}

void connection::on_write(){
    if (!outbound.size())
        return;

    ssize_t sent;
    try {
        sent = sock.send(
            outbound.data(), outbound.size()
        );
    } catch (const failed_send &){
        state = dead;
        outbound.clear();
        return;
    }

    if (sent < 0)
        return;

    outbound.erase(
        outbound.begin(), outbound.begin() + sent
    );
}

bool connection::is_dead(){
    return state == dead;
}

bool connection::has_outbound_data(){
    return !!outbound.size();
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
                packet.intent
                == packet_intention::intent_status
            )
                state = status_request;
            else if (
                packet.intent
                == packet_intention::intent_login
            )
                state = login_start;

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

        case login_start: {

        }

        case login_success: {

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
