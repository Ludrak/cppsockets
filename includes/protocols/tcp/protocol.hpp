#pragma once

#include <sys/socket.h>
#include <arpa/inet.h>
#include "common/protocols/protocol.hpp"
#include <iostream>

#include "server/server_client.hpp"
#include "client/client_connection.hpp"


struct TCPData {  public: int dataTCP; };

template<>
class ProtocolMethod<Side::SERVER, Protocols::Transport::TCP> : public IProtocolMethod<Side::SERVER>
{
    public:
        typedef TCPData  data_type;
        typedef GatewayInterface<Side::SERVER, Protocols::Transport::TCP>  interface_type;

        ProtocolMethod();

        int generateSocket() const override;

        #define RECV_BLK_SIZE 1024
        int receiveMethod(IServerClient* client) override;

        int sendMethod(IServerClient* client, const void *data, const size_t size) override;

        int acceptMethod(const int endpoint_socket, void *addr_buffer, socklen_t* addr_len) override;
};

template<>
class ProtocolMethod<Side::CLIENT, Protocols::Transport::TCP> : public IProtocolMethod<Side::CLIENT>
{
    public:
        typedef TCPData  data_type;
        typedef GatewayInterface<Side::CLIENT, Protocols::Transport::TCP>   interface_type;

        ProtocolMethod();

        int generateSocket() const override;

        #define RECV_BLK_SIZE 1024
        int receiveMethod(IClientConnection* connection) override;
        
        int sendMethod(IClientConnection* client, const void *data, const size_t size) override;
};
