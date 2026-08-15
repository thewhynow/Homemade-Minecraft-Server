#pragma once
#include <exception>
#include <stdexcept>

class parse_error : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
    parse_error();
};

class malformed_packet : public parse_error {
public:
    using parse_error::parse_error;
    malformed_packet();
};

class unfinished_packet : public parse_error {
public:
    using parse_error::parse_error;
    unfinished_packet();
};

class nbt_end_tag : public parse_error {

};

class network_error : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

class failed_accept : public network_error {
public:
    using network_error::network_error;
    failed_accept();
};

class failed_read : public network_error {
public:
    using network_error::network_error;
    failed_read();
};

class failed_send : public network_error {
public:
    using network_error::network_error;
    failed_send();
};

class closed_connection : public network_error {
public:
    using network_error::network_error;
    closed_connection();
};

class failed_getaddrinfo : public network_error {
public:
    using network_error::network_error;
    failed_getaddrinfo();
};

class failed_bind : public network_error {
public:
    using network_error::network_error;
    failed_bind();
};

class failed_listen : public network_error {
public:
    using network_error::network_error;
    failed_listen();
};
