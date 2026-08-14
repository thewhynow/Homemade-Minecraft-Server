#include "inc/packet.hpp"

packet::packet(
    std::span<uint8_t> &buff
):
    id(buff)
{}

packet::packet(
    net_var_int id
):
    id(id)
{}

packet_intention::packet_intention(
    std::span<uint8_t> &buff
):
    packet(buff),
    version(buff),
    address(buff, 255),
    port(buff),
    intent(buff)
{}

packet_status_response::packet_status_response(
    std::string_view json
):
    packet(0x00),
    json_response(json)
{}

void packet_status_response::serialize(
    std::vector<uint8_t> &buff
) const {
    net_var_int(
        packet::id.size() + json_response.size()
    ).serialize(buff);

    packet::id.serialize(buff);
    json_response.serialize(buff);
}

packet_pong_response::packet_pong_response(
    net_long timestamp
):
    packet(0x01),
    timestamp(timestamp)
{}

void packet_pong_response::serialize (
    std::vector<uint8_t> &buff
) const {
    net_var_int(
        packet::id.size() + timestamp.size()
    ).serialize(buff);

    packet::id.serialize(buff);
    timestamp.serialize(buff);
}

packet_status_request::packet_status_request(
    std::span<uint8_t> &buff
):
    packet(buff)
{}

packet_ping_request::packet_ping_request(
    std::span<uint8_t> &buff
):
    packet(buff),
    timestamp(buff)
{}


