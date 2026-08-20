#include "inc/connection.hpp"
#include <sys/socket.h>
#include <unistd.h>
#include <span>
#include <cstdlib>
#include "inc/packet.hpp"
#include "inc/errors.hpp"
#include "inc/registry.hpp"

#include <print>

template <>
void connection::queue_packet<packet_registry_data>(
    const packet_registry_data &packet
);

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
                packet.intent()
                == packet_intention::intent_status
            )
                state = status_request;
            else if (
                packet.intent()
                == packet_intention::intent_login
            )
                state = login_start;

            break;
        }

        case status_request: {
            packet_status_request packet(buff);

            packet_status_response response(
                {(uint8_t) packet_id::status::response},
                {
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
                }
            );

            queue_packet(response);
            state = status_ping;
            break;
        }

        case status_ping: {
            packet_ping_request packet(buff);

            packet_pong_response response(
                {(uint8_t) packet_id::status::pong},
                {packet.timestamp()}
            );

            queue_packet(response);
            state = dead;
            break;
        }

        case login_start: {
            packet_hello packet(buff);

            std::println("made it login_start");

            packet_login_finished response {
                {(uint8_t) packet_id::login::finished},
                {
                    packet.player_uuid(),
                    packet.name(),
                    {{}}
                },
                {0, 0}
            };

            queue_packet(response);
            state = login_success;
            break;
        }

        case login_success: {
            packet_login_acknowledged packet(buff);

            std::println("made it login_success");

            packet_select_known_packs response {
                {(uint8_t) packet_id::configuration::known_client_bound},
                {
                    {
                        {{"minecraft"}, {"core"}, {"26.2"}}
                    }
                }
            };

            queue_packet(response);
            state = configuration_select;
            break;
        }

        case configuration_select: {
            /**
             * client sending packets we don't handle -> bad parsing -> freeze
             **/
            packet_select_known_packs packet(buff);

            std::println("made it here to config_select");

            for (const auto &i : packet.known_packs().data)
                if (
                    i.name_space().value == "minecraft" &&
                    i.id().value == "core" &&
                    i.version().value == "26.2"
                )
                    goto success;

            throw std::runtime_error(
                "client does not have minecraft:core/26.2"
            );

        success:
            synced_registries::instance.queue_packets(
                [this](const packet_registry_data &packet) -> void {
                    queue_packet<packet_registry_data>(packet);
                }
            );

            state = configuration_registry;
            break;
        }

        case configuration_registry: {
            packet_finish_configuration packet (
                (uint8_t) packet_id::configuration::finish
            );

            queue_packet(packet);

            state = configuration_finish;
            break;
        }

        case configuration_finish: {
            packet_finish_configuration packet(buff);

            state = play;
            break;
        }

        case play: {
            std::println("made it here");
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

template <>
void connection::queue_packet<packet_registry_data>(
    const packet_registry_data &packet
){
    std::vector<uint8_t> serialized;
    packet.serialize(serialized);
    outbound.insert(
        outbound.end(), 
        serialized.begin(), 
        serialized.end()
    );

    std::println("sending registry {}", packet.id().value);
}
