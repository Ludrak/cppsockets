#include "protocols/tcp/protocol.hpp"

ProtocolMethod<Protocol::TCP>::ProtocolMethod()
: ProtocolBase()
{}

int ProtocolMethod<Protocol::TCP>::generateSocket() const
{
    // TODO: proper family specification
    return (::socket(AF_INET, SOCK_STREAM, 0));
}

int ProtocolMethod<Protocol::TCP>::receiveMethod(const int socket, void *data, const size_t size)
{
    ssize_t recv_bytes = recv(socket, data, size, MSG_DONTWAIT);
    if (recv_bytes < 0)
        return -1;
    else if (recv_bytes == 0)
        return 0;
    
    return recv_bytes;
}

int ProtocolMethod<Protocol::TCP>::sendMethod(const int socket, const void *data, const size_t size)
{
    ssize_t sent_bytes = send(socket, data, size, MSG_DONTWAIT);
    if (sent_bytes < 0)
        return -1;
    else if (static_cast<size_t>(sent_bytes) < size)
        return sent_bytes;
    
    return 0;
}

int ProtocolMethod<Protocol::TCP>::acceptMethod(const int endpoint_socket, void* addr_buffer, socklen_t* addr_len)
{
    // TODO: proper family specification
    int socket = ::accept(endpoint_socket, reinterpret_cast<sockaddr*>(addr_buffer), addr_len);
    if (socket < 0)
        return (-1);
    
    return (socket);
}