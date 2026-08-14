#include "inc/socket.hpp"
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <exception>
#include <cassert>

socket_wrapper::socket_wrapper(int sfd){
    struct sockaddr_storage their_addr;
    socklen_t their_size;

    fd = ::accept(
        sfd, 
        reinterpret_cast<struct sockaddr*>(&their_addr),
        &their_size
    );
}

socket_wrapper::socket_wrapper(
    socket_wrapper &&other
):
    fd(other.fd)
{
    other.fd = -1;
}

socket_wrapper &socket_wrapper::operator=(
    socket_wrapper &&other
){
    fd = other.fd;
    other.fd = -1;

    return *this;
}

socket_wrapper::~socket_wrapper(){
    close(fd);
}

size_t socket_wrapper::send(
    const void *data, size_t len
){
    ssize_t res = ::send(fd, data, len, 0);

    if (res == -1)
        throw std::exception();

    return res;
}

size_t socket_wrapper::recv(
    void *data, size_t len
){
    ssize_t res = ::recv(fd, data, len, 0);

    /* this is unlikely since we are using blocking sockets */
    if (res == -1)
        throw std::exception();

    return res;
}
