#pragma once

#include <cstdlib>
#include <sys/socket.h>
#include "common/protocols/protocol_list.hpp"
#include "common/side.hpp"
// #include "client/client_connection.hpp"
// #include "server/server_client.hpp"
#include "common/data/data.hpp"

// Predefinition of GatewayInterface class
template<Side S, class Proto>
class GatewayInterface;

// Predefinition of ServerClient class
template<class ...T>
class ServerClient;
class IServerClient;

// Predefintion of ServerEndpoint class
template<class Proto>
class ServerEndpoint;
class IServerEndpoint;

// Predefinition of ClientConnection class
template<class Proto, class ...T>
class ClientConnection;
class IClientConnection;

// Predefintion of Client class
class Client;

// Predefinition of Server class
class Server;


// TODO: change client reference type to ServerClient
// TODO: add data type aggregation (ex: ProtocolMethod<Protocol::TLS> will need a custom data containing accept_done, certificates, SSL*, etc..)
template<Side S>
class IProtocolMethod {};

template<>
class IProtocolMethod<Side::SERVER>
{
    public:
        virtual ~IProtocolMethod() {}

        // Generates a socket of the type that should be used for this protocol
        // returns:
        // -1           : socket failed
        // >0           : new socket file descriptor
        virtual int            generateSocket() const = 0;
        
        // returns:
        // -1   : an error has occured, disconnect the client
        //  0   : the client disconnected
        // >0   : received data
        virtual int             receiveMethod(IServerClient* client) = 0;

        // you must specify a data array at least as long as the specified size (in bytes)
        // sendMethod will send the content pointed by data to the specified socket
        // returns:
        // -1   : an error has occured, the data was not sent
        //  0   : send has been able to send the whole provided data, remove clientWantsWrite from SocketHandler
        // >0   : send did not sent the data entierly, the size of the sent data is returned, dont remove clientWantsWrite from SocketHandler
        virtual int             sendMethod(IServerClient* client, const void *data, const size_t size) = 0;

        // will try to accept a new incomming connection from the endpoint_socket
        // addr_buffer needs to be at least as long as the value pointed by addr_size
        // addr_buffer and addr_size will be filled by the sockaddr struct receive
        // TODO: addr size must be managed along with socket family types (i.e. sockaddr_in vs sockaddr_in6)
        //       THIS METHOD SHOULD ABSTRACT ALL SOCKADDR TYPES
        // returns:
        // -1           : an error has occured, unable to accept the client
        // >0           : accepted client socket file descriptor
        virtual int             acceptMethod(const int endpoint_socket, void *addr_buffer, socklen_t *addr_size) = 0;
};



template<>
class IProtocolMethod<Side::CLIENT>
{
    public:
        virtual ~IProtocolMethod() {}

        // Generates a socket of the type that should be used for this protocol
        // returns:
        // -1           : socket failed
        // >0           : new socket file descriptor
        virtual int            generateSocket() const = 0;
        
        // you must specify a data_buffer at least as long as the specified size (in bytes)
        // receiveMethod will fill the data_buffer with the received data on socket
        // returns:
        // -1   : an error has occured, disconnect the client
        //  0   : the client disconnected
        // >0   : received data
        virtual int             receiveMethod(IClientConnection* client) = 0;

        // you must specify a data array at least as long as the specified size (in bytes)
        // sendMethod will send the content pointed by data to the specified socket
        // returns:
        // -1   : an error has occured, the data was not sent
        //  0   : send has been able to send the whole provided data, remove clientWantsWrite from SocketHandler
        // >0   : send did not sent the data entierly, the size of the sent data is returned, dont remove clientWantsWrite from SocketHandler
        virtual int             sendMethod(IClientConnection* client, const void *data, const size_t size) = 0;
};

template<Side S, class Proto>
class ProtocolMethod
{
};










/* PSEUDO TCP IMPLEMENTATION  */


// class TCPDataType
// {
//     public:
//         int a;
// };

// template<>
// class ProtocolMethod<Side::SERVER, Protocols::Transport::TCP> : IProtocolMethod<Side::SERVER>
// {
//     public:
//         typedef TCPDataType  data_type;

//         int             generateSocket() const override
//         {

//         }
        
//         int             receiveMethod(IServerClient* client, void *data_buffer, const size_t size) override
//         {
//             GatewayInterface<Side::SERVER, Protocols::Transport::TCP>::client_type* c = reinterpret_cast<GatewayInterface<Side::SERVER, Protocols::Transport::TCP>::client_type*>(client);
//             c->data.a = 1;
//         }

//         int             sendMethod(IServerClient* client, const void *data, const size_t size) override
//         {

//         }

//         int acceptMethod(const int endpoint_socket, void *addr_buffer, socklen_t *addr_size) override
//         {

//         }
// };


// template<>
// class ProtocolMethod<Side::CLIENT, Protocols::Transport::TCP> : IProtocolMethod<Side::CLIENT>
// {
//     public:
//         typedef TCPDataType  data_type;

      
//         int             generateSocket() const override
//         {

//         }
        
//         int             receiveMethod(IClientConnection* client, void *data_buffer, const size_t size) override
//         {
//             GatewayInterface<Side::CLIENT, Protocols::Transport::TCP>::client_type* c = reinterpret_cast<GatewayInterface<Side::CLIENT, Protocols::Transport::TCP>::client_type*>(client);
//             c->data.a = 1;
//             // client.data.a = 10;
//             // client.data.b = 43;
//         }

//         int             sendMethod(IClientConnection* client, const void *data, const size_t size) override
//         {

//         }
// };


