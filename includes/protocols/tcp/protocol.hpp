#pragma once

#include <sys/socket.h>
#include <arpa/inet.h>
#include "common/protocols/protocol.hpp"

template<>
class ProtocolMethod<Protocol::TCP> : public ProtocolBase
{
    public:
        ProtocolMethod();

        int generateSocket() const override;

        int receiveMethod(const int socket, void *data_buffer, const size_t size) override;
        
        int sendMethod(const int socket, const void *data, const size_t size) override;

        int acceptMethod(const int endpoint_socket, void *addr_buffer, socklen_t* addr_len) override;
};