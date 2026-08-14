#include "inc/server.hpp"

server server::instance;

server::server() = default;

void server::start(){
    set_main_fd();

    /* pollfds[0] is the main socket */
    pollfds.emplace_back(sfd, POLLIN, 0);
    /* push an empty connection so they are the same length */
    connections.push_back(connection(socket_wrapper(0)));

    while (true){
        poll(pollfds.data(), pollfds.size(), -1);

        if (pollfds[0].revents & POLLIN)
            accept_connection();

        for (size_t i = 1; i < pollfds.size(); ++i){
            if (connections[i].is_dead())
                continue;

            if (pollfds[i].revents & POLLIN)
                connections[i].on_read();

            if (pollfds[i].revents & POLLOUT)
                connections[i].on_write();
        }

        remove_connections();
    }
}

void server::set_main_fd(){
    addrinfo hints = {
        .ai_family   = AF_UNSPEC,
        .ai_socktype = SOCK_STREAM,
        .ai_flags    = AI_PASSIVE
    }, 
    *res;

    int err = getaddrinfo(NULL, "25565", &hints, &res);

    if (err == -1)
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
            fd = -1;
            close(fd);
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
    sockaddr_storage their_addr;
    socklen_t their_len;
    int nfd = accept(
        sfd, (sockaddr*) &their_addr, &their_len
    );

    if (nfd == -1)
        throw std::exception();

    set_nonblocking(nfd);

    pollfds.emplace_back(
        nfd, POLLIN | POLLOUT, 0
    );
    connections.emplace_back(socket_wrapper(nfd));
}

void server::remove_connections(){
    for (size_t i = 0; i < connections.size(); ++i){
        if (!connections[i].is_dead())
            continue;

        pollfds.erase(pollfds.begin() + i);
        connections.erase(connections.begin() + i);
    }
}

void server::set_nonblocking(int fd){
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1)
        throw std::exception();
    if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1)
        throw std::exception();
}
