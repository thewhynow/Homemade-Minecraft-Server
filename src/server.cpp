#include "inc/server.hpp"

server server::instance;

server::server() = default;

void server::start(){
    set_main_fd();

    while (true){
        pollfds.insert(pollfds.begin(), spollfd);

        /* 20 ticks / s -> 50 ms / tick */
        int err = poll(pollfds.data(), pollfds.size(), 50);
        if (err < 0)
            throw std::runtime_error("poll(2) failed");

        if (pollfds[0].revents & POLLIN)
            try {
                accept_connection();
            }
            catch (const failed_accept &)
            {}

        pollfds.erase(pollfds.begin());

        for (size_t i = 0; i < pollfds.size() - 1; ++i){
            if (pollfds[i].revents & POLLOUT){
                connections[i].on_write();

                if (!connections[i].has_outbound_data())
                    pollfds[i].events &= ~POLLOUT;
            }

            if (connections[i].is_dead())
                continue;

            if (pollfds[i].revents & POLLIN)
                connections[i].on_read();

            if (connections[i].has_outbound_data())
                pollfds[i].events |= POLLOUT;
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
        throw std::runtime_error(
            std::string("getaddrinfo") + gai_strerror(err)
        );

    int fd = -1;
    for (auto i = res; i; i = i->ai_next){
        fd = socket(
            i->ai_family, i->ai_socktype, i->ai_protocol
        );

        if (fd == -1)
            continue;

        int yes = 1;
        setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof yes);

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
        throw std::runtime_error(
            std::string("bind: ") + strerror(errno)
        );

    if (listen(fd, 10) == -1)
        throw std::runtime_error(
            std::string("listen: ") + strerror(errno)
        );

    spollfd = pollfd{fd, POLLIN, 0};
}

void server::accept_connection(){
    socket_wrapper sock(spollfd.fd);
    pollfds.emplace_back(sock.get_fd(), POLLIN, 0);
    connections.emplace_back(std::move(sock));
}

void server::remove_connections(){
    for (size_t i = 0; i < connections.size(); ++i){
        if (
            !connections[i].is_dead() ||
            connections[i].has_outbound_data()
        )
            continue;

        pollfds.erase(pollfds.begin() + i + 1);
        connections.erase(connections.begin() + i);
        --i;
    }
}
