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
    port(net_type_from_span<net_ushort>(buff)),
    intent(buff)
{}

packet_status_response::packet_status_response(
    std::string_view json
):
    packet{net_var_int(0)},
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
        packet::id.size() + sizeof timestamp
    ).serialize(buff);

    packet::id.serialize(buff);
    net_type_serialize(timestamp, buff);
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
    timestamp(net_type_from_span<net_long>(buff))
{}


