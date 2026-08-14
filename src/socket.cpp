#include "inc/socket.hpp"
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <exception>
#include <cassert>
#include "inc/errors.hpp"
#include <errno.h>

socket_wrapper::socket_wrapper(int sfd){
    struct sockaddr_storage their_addr;
    socklen_t their_size = sizeof their_addr;

    fd = ::accept(
        sfd, 
        reinterpret_cast<struct sockaddr*>(&their_addr),
        &their_size
    );

    if (fd == -1)
        throw failed_accept();

    set_nonblocking();
}

socket_wrapper::socket_wrapper(socket_wrapper &&other):
    fd(other.fd)
{
    other.fd = -1;
}

socket_wrapper &socket_wrapper::operator=(
    socket_wrapper &&other
){
    if (fd != -1)
        close(fd);

    fd = other.fd;
    other.fd = -1;

    return *this;
}

socket_wrapper::~socket_wrapper(){
    if (fd != -1)
        close(fd);
}

ssize_t socket_wrapper::send(
    const void *data, size_t len
){
    ssize_t res = ::send(fd, data, len, 0);

    if (res == -1){
        if (errno == EAGAIN)
            return -1;
        else
            throw failed_send();
    }

    return res;
}

ssize_t socket_wrapper::recv(
    void *data, size_t len
){
    ssize_t res = ::recv(fd, data, len, 0);

    if (res == -1){
        if (errno == EAGAIN)
            return -1;
        else
            throw failed_read();
    }

    return res;
}

int socket_wrapper::get_fd(){
    return fd;
}

void socket_wrapper::set_nonblocking(){
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1)
        throw std::exception();
    if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1)
        throw std::exception();
}
