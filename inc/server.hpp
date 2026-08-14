#pragma once
#include <poll.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <poll.h>
#include <vector>
#include "connection.hpp"

class server {
private:
    server();
public:
    void start();
    static server instance;
private:
    void set_main_fd();
    void accept_connection();
    void remove_connections();
    void set_nonblocking(int fd);
private:
    int sfd;
    std::vector<pollfd> pollfds;
    std::vector<connection> connections;
};
