#include "inc/server.hpp"

server server::instance;

server::server() = default;

void server::start(){
    set_main_fd();

    /* pollfds[0] is the main socket */
    pollfds.emplace_back(sfd, POLLIN, 0);

    while (true){
        poll(pollfds.data(), pollfds.size(), -1);

        if (pollfds[0].revents & POLLIN)
            accept_connection();

        for (size_t i = 1; i < pollfds.size(); ++i){
            if (connections[i - 1].is_dead())
                continue;

            if (pollfds[i].revents & POLLIN)
                connections[i - 1].on_read();

            if (pollfds[i].revents & POLLOUT)
                connections[i - 1].on_write();
        }

        remove_connections();
    }
}

void server::set_main_fd(){
    addrinfo hints = {
        .ai_flags    = AI_PASSIVE,
        .ai_family   = AF_UNSPEC,
        .ai_socktype = SOCK_STREAM
    }, 
    *res;

    int err = getaddrinfo(NULL, "25565", &hints, &res);

    if (err != 0)
        throw std::exception();

    int fd = -1;
    for (auto i = res; i; i = i->ai_next){
        fd = socket(
            i->ai_family, i->ai_socktype, i->ai_protocol
        );

        if (fd == -1)
            continue;

        int res = bind(fd, i->ai_addr, i->ai_addrlen);
        if (res == -1){
            close(fd);
            fd = -1;
            continue;
        }
        else break;
    }

    freeaddrinfo(res);

    if (fd == -1)
        throw std::exception();

    if (listen(fd, 10) == -1)
        throw std::exception();

    sfd = fd;
}

void server::accept_connection(){
    socket_wrapper sock(sfd);
    pollfds.emplace_back(
        sock.get_fd(), POLLIN | POLLOUT, 0
    );
    connections.emplace_back(std::move(sock));
}

void server::remove_connections(){
    for (size_t i = 0; i < connections.size(); ++i){
        if (!connections[i].is_dead())
            continue;

        pollfds.erase(pollfds.begin() + i + 1);
        connections.erase(connections.begin() + i);
        --i;
    }
}
