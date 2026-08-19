#include "inc/connection.hpp"
#include <sys/socket.h>
#include <unistd.h>
#include <span>
#include <cstdlib>
#include "inc/packet.hpp"
#include "inc/errors.hpp"
#include <print>

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
            packet_select_known_packs packet(buff);

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
            std::vector<net_registry_data_entry> entries;
            /* deduction actually works here */
            entries.push_back({{"minecraft:pattern_item/bordure_indented"}, {nullptr}});
            entries.push_back({{"minecraft:pattern_item/creeper"}, {nullptr}});
            entries.push_back({{"minecraft:pattern_item/field_masoned"}, {nullptr}});
            entries.push_back({{"minecraft:pattern_item/flow"}, {nullptr}});
            entries.push_back({{"minecraft:pattern_item/flower"}, {nullptr}});
            entries.push_back({{"minecraft:pattern_item/globe"}, {nullptr}});
            entries.push_back({{"minecraft:pattern_item/guster"}, {nullptr}});
            entries.push_back({{"minecraft:pattern_item/mojang"}, {nullptr}});
            entries.push_back({{"minecraft:pattern_item/piglin"}, {nullptr}});
            entries.push_back({{"minecraft:pattern_item/skull"}, {nullptr}});
            queue_registry("minecraft:banner_pattern", std::move(entries));

            queue_registry("minecraft:chat_type", std::move(entries));

            entries.push_back({{"minecraft:cactus"}, {nullptr}});
            entries.push_back({{"minecraft:campfire"}, {nullptr}});
            entries.push_back({{"minecraft:cramming"}, {nullptr}});
            entries.push_back({{"minecraft:dragon_breath"}, {nullptr}});
            entries.push_back({{"minecraft:drown"}, {nullptr}});
            entries.push_back({{"minecraft:dry_out"}, {nullptr}});
            entries.push_back({{"minecraft:ender_pearl"}, {nullptr}});
            entries.push_back({{"minecraft:fall"}, {nullptr}});
            entries.push_back({{"minecraft:fly_into_wall"}, {nullptr}});
            entries.push_back({{"minecraft:freeze"}, {nullptr}});
            entries.push_back({{"minecraft:generic"}, {nullptr}});
            entries.push_back({{"minecraft:generic_kill"}, {nullptr}});
            entries.push_back({{"minecraft:hot_floor"}, {nullptr}});
            entries.push_back({{"minecraft:in_fire"}, {nullptr}});
            entries.push_back({{"minecraft:in_wall"}, {nullptr}});
            entries.push_back({{"minecraft:lava"}, {nullptr}});
            entries.push_back({{"minecraft:lightning_bolt"}, {nullptr}});
            entries.push_back({{"minecraft:magic"}, {nullptr}});
            entries.push_back({{"minecraft:on_fire"}, {nullptr}});
            entries.push_back({{"minecraft:out_of_world"}, {nullptr}});
            entries.push_back({{"minecraft:outside_border"}, {nullptr}});
            entries.push_back({{"minecraft:stalagmite"}, {nullptr}});
            entries.push_back({{"minecraft:starve"}, {nullptr}});
            entries.push_back({{"minecraft:sweet_berry_bush"}, {nullptr}});
            entries.push_back({{"minecraft:wither"}, {nullptr}});
            queue_registry("minecraft:damage_type", std::move(entries));

            queue_registry("minecraft:dialog", std::move(entries));

            entries.push_back({{"minecraft:overworld"}, {nullptr}});
            queue_registry("minecraft:dimension_type", std::move(entries));


            state = configuration_registry;
            break;
        }

        case play: {

        }
    }
}

void connection::queue_registry(
    std::string_view name, 
    std::vector<net_registry_data_entry> 
        &&entries
){
    queue_packet(
        packet_registry_data(
            {(uint8_t) packet_id::configuration::registry},
            {name},
            {std::move(entries)}
        )
    );
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
