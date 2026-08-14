#include "inc/errors.hpp"

parse_error::parse_error():
    std::runtime_error("parsing error")
{}

malformed_packet::malformed_packet():
    parse_error("malformed packet")
{}

unfinished_packet::unfinished_packet():
    parse_error("packet not finished")
{}

failed_accept::failed_accept():
    network_error("accept(2) failed")
{}

failed_read::failed_read():
    network_error("read(2) failed")
{}

failed_send::failed_send():
    network_error("send(2) failed")
{}

closed_connection::closed_connection():
    network_error("connection closed")
{}

failed_getaddrinfo::failed_getaddrinfo():
    failed_getaddrinfo("getaddrinfo(2) failed")
{}

failed_bind::failed_bind():
    failed_bind("failed to find bindable socket")
{}

failed_listen::failed_listen():
    failed_listen("listen(2) failed")
{}

