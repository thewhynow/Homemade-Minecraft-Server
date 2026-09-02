#include "inc/connection.hpp"
#include <sys/socket.h>
#include <unistd.h>
#include <span>
#include <cstdlib>
#include "inc/packet.hpp"
#include "inc/errors.hpp"
#include "inc/registry.hpp"
#include "inc/types.hpp"

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
            state = configuration;
            break;
        }

        case configuration:
        case configuration_finish: {
            handle_configuration(buff);
            break;
        }

        case play: {
            std::println("made it to play");
        }
    }
}

void connection::handle_configuration(
    std::span<uint8_t> &buff
){
    packet_id::configuration id = (packet_id::configuration) buff[0];

    switch (id){
        case packet_id::configuration::custom_server_bound: {
            packet_custom_payload_plugin_message packet{buff};

            std::println("custom server bound brand packet");

            packet_custom_payload_plugin_message response {
                (uint8_t) packet_id::configuration::custom_client_bound,
                {"minecraft:brand"},
                {"thewhynow"}
            };

            queue_packet(response);

            break;
        };

        case packet_id::configuration::known_server_bound: {
            packet_select_known_packs packet{buff};

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
                    queue_packet(packet);
                }
            );

            const registry &banner_pattern_reg =
                synced_registries::instance["banner_pattern"]
            ;
            const registry &damage_type_reg =
                synced_registries::instance["damage_type"]
            ;
            const registry &timeline_reg =
                synced_registries::instance["timeline"]
            ;

            queue_packet(
                packet_update_tags {
                    (uint8_t) packet_id::configuration::update_tags,
                    {
                        {
                            {
                                {"minecraft:banner_pattern"},
                                {{
                                    {
                                        {"minecraft:pattern_item/bordure_indented"},
                                        {{{banner_pattern_reg["minecraft:curly_border"]}}}
                                    },
                                    {
                                        {"minecraft:pattern_item/creeper"},
                                        {{{banner_pattern_reg["minecraft:creeper"]}}}
                                    },
                                    {
                                        {"minecraft:pattern_item/field_masoned"},
                                        {{{banner_pattern_reg["minecraft:bricks"]}}}
                                    },
                                    {
                                        {"minecraft:pattern_item/flow"},
                                        {{{banner_pattern_reg["minecraft:flow"]}}}
                                    },
                                    {
                                        {"minecraft:pattern_item/flower"},
                                        {{{banner_pattern_reg["minecraft:flower"]}}}
                                    },
                                    {
                                        {"minecraft:pattern_item/globe"},
                                        {{{banner_pattern_reg["minecraft:globe"]}}}
                                    },
                                    {
                                        {"minecraft:pattern_item/guster"},
                                        {{{banner_pattern_reg["minecraft:guster"]}}}
                                    },
                                    {
                                        {"minecraft:pattern_item/mojang"},
                                        {{{banner_pattern_reg["minecraft:mojang"]}}}
                                    },
                                    {
                                        {"minecraft:pattern_item/piglin"},
                                        {{{banner_pattern_reg["minecraft:piglin"]}}}
                                    },
                                    {
                                        {"minecraft:pattern_item/skull"},
                                        {{{banner_pattern_reg["minecraft:skull"]}}}
                                    },
                                }}
                            },
                            {
                                {"minecraft:damage_type"},
                                {{
                                    {
                                        {"minecraft:bypasses_shield"},
                                        {{
                                            {damage_type_reg["minecraft:cramming"]},
                                            {damage_type_reg["minecraft:dragon_breath"]},
                                            {damage_type_reg["minecraft:drown"]},
                                            {damage_type_reg["minecraft:ender_pearl"]},
                                            {damage_type_reg["minecraft:fall"]},
                                            {damage_type_reg["minecraft:fly_into_wall"]},
                                            {damage_type_reg["minecraft:freeze"]},
                                            {damage_type_reg["minecraft:generic"]},
                                            {damage_type_reg["minecraft:generic_kill"]},
                                            {damage_type_reg["minecraft:in_wall"]},
                                            {damage_type_reg["minecraft:magic"]},
                                            {damage_type_reg["minecraft:on_fire"]},
                                            {damage_type_reg["minecraft:out_of_world"]},
                                            {damage_type_reg["minecraft:outside_border"]},
                                            {damage_type_reg["minecraft:stalagmite"]},
                                            {damage_type_reg["minecraft:starve"]},
                                            {damage_type_reg["minecraft:wither"]},
                                            {damage_type_reg["minecraft:cactus"]},
                                            {damage_type_reg["minecraft:campfire"]},
                                            {damage_type_reg["minecraft:dry_out"]},
                                            {damage_type_reg["minecraft:hot_floor"]},
                                            {damage_type_reg["minecraft:in_fire"]},
                                            {damage_type_reg["minecraft:lava"]},
                                            {damage_type_reg["minecraft:lightning_bolt"]},
                                            {damage_type_reg["minecraft:sweet_berry_bush"]},
                                        }}
                                    },
                                    {
                                        {"minecraft:is_explosion"},
                                        {{/* empty */}}
                                    },
                                    {
                                        {"minecraft:is_fire"},
                                        {{
                                            {damage_type_reg["minecraft:campfire"]},
                                            {damage_type_reg["minecraft:hot_floor"]},
                                            {damage_type_reg["minecraft:in_fire"]},
                                            {damage_type_reg["minecraft:lava"]},
                                            {damage_type_reg["minecraft:on_fire"]},
                                        }}
                                    }
                                }}
                            },
                            {
                                {"minecraft:timeline"},
                                {{
                                    {
                                        {"minecraft:in_overworld"},
                                        {{
                                            {timeline_reg["minecraft:day"]},
                                            {timeline_reg["minecraft:moon"]},
                                            {timeline_reg["minecraft:early_game"]},
                                            {timeline_reg["minecraft:villager_schedule"]},
                                        }}
                                    }
                                }}
                            },
                            /*
                                entries of tags on built-in registries are
                                numeric ids, taken from reports/registries.json
                                (26.2). they must be regenerated on a version
                                bump.
                            */
                            {
                                {"minecraft:block"},
                                {{
                                    {
                                        {"minecraft:infiniburn_overworld"},
                                        {{
                                            {285}, /* minecraft:netherrack */
                                            {671}, /* minecraft:magma_block */
                                        }}
                                    }
                                }}
                            },
                            {
                                {"minecraft:item"},
                                {{
                                    {
                                        {"minecraft:sulfur_cube_archetype/regular"},
                                        {{
                                            {658}, /* minecraft:white_concrete_powder */
                                            {659}, /* minecraft:orange_concrete_powder */
                                            {660}, /* minecraft:magenta_concrete_powder */
                                            {661}, /* minecraft:light_blue_concrete_powder */
                                            {662}, /* minecraft:yellow_concrete_powder */
                                            {663}, /* minecraft:lime_concrete_powder */
                                            {664}, /* minecraft:pink_concrete_powder */
                                            {665}, /* minecraft:gray_concrete_powder */
                                            {666}, /* minecraft:light_gray_concrete_powder */
                                            {667}, /* minecraft:cyan_concrete_powder */
                                            {668}, /* minecraft:purple_concrete_powder */
                                            {669}, /* minecraft:blue_concrete_powder */
                                            {670}, /* minecraft:brown_concrete_powder */
                                            {671}, /* minecraft:green_concrete_powder */
                                            {672}, /* minecraft:red_concrete_powder */
                                            {673}, /* minecraft:black_concrete_powder */
                                            {59}, /* minecraft:mud */
                                            {171}, /* minecraft:muddy_mangrove_roots */
                                            {407}, /* minecraft:packed_mud */
                                            {110}, /* minecraft:coal_block */
                                            {55}, /* minecraft:dirt */
                                            {56}, /* minecraft:coarse_dirt */
                                            {58}, /* minecraft:rooted_dirt */
                                            {57}, /* minecraft:podzol */
                                            {54}, /* minecraft:grass_block */
                                            {370}, /* minecraft:clay */
                                            {607}, /* minecraft:bone_block */
                                        }}
                                    }
                                }}
                            },
                            {
                                {"minecraft:worldgen/biome"},
                                {{
                                    {
                                        {"minecraft:spawns_cold_variant_farm_animals"},
                                        {{/* empty */}}
                                    },
                                    {
                                        {"minecraft:spawns_warm_variant_farm_animals"},
                                        {{/* empty */}}
                                    }
                                }}
                            },
                        }
                    }
                }
            );

            packet_finish_configuration response {
                (uint8_t) packet_id::configuration::finish
            };

            queue_packet(response);

            std::println("sending registry packets");

            state = configuration_finish;
            break;
        }

        case packet_id::configuration::client_information: {
            packet_client_information packet(buff);

            std::println("recieved client information");

            break;
        }

        case packet_id::configuration::finish: {
            packet_finish_configuration packet(buff);

            std::println("play state activated");

            state = play;
            break;
        }

        default: {
            throw std::runtime_error("bad configuration packet");
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
