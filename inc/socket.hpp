#pragma once
#include <poll.h>
#include <cstdlib>

class socket_wrapper {
public:
    socket_wrapper(int sfd);
    socket_wrapper(socket_wrapper &&other);
    socket_wrapper &operator=(socket_wrapper &&other);
    ~socket_wrapper();

    size_t send(const void *data, size_t len);
    size_t recv(void *data, size_t len);
private:
    int fd;
};
