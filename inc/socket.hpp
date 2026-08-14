#pragma once
#include <poll.h>
#include <cstdlib>
#include <unistd.h>

class socket_wrapper {
public:
    socket_wrapper(int sfd);
    socket_wrapper(socket_wrapper &&other);
    socket_wrapper &operator=(socket_wrapper &&other);
    ~socket_wrapper();

    ssize_t send(const void *data, size_t len);
    ssize_t recv(void *data, size_t len);

    int get_fd();
    void set_nonblocking();
private:
    int fd;
};
