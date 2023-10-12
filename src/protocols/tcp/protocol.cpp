#include "protocols/tcp/interface.hpp"


/**********************************************************************/
/**                GATEWAY INTERFACE (SERVER SIDE)                   **/
/**********************************************************************/

GatewayInterface<Side::SERVER, Protocols::Transport::TCP>::GatewayInterface(Server& server)
: server(server), endpoints(), _protocol()
{}

IProtocolMethod<Side::SERVER>*     GatewayInterface<Side::SERVER, Protocols::Transport::TCP>::getProtocol()
{
    return (reinterpret_cast<IProtocolMethod<Side::SERVER>*>(&this->_protocol));
}

void    GatewayInterface<Side::SERVER, Protocols::Transport::TCP>::attachToEndpoint(IServerEndpoint* endpoint)
{
    endpoint_type* ep = dynamic_cast<endpoint_type*>(endpoint);
    if (ep == nullptr)
        return ;
    this->endpoints.push_back(ep);
}

void    GatewayInterface<Side::SERVER, Protocols::Transport::TCP>::detachFromEndpoint(const int socket)
{
    for (std::vector<endpoint_type*>::iterator it = this->endpoints.begin(); it != this->endpoints.end(); ++it)
    {
        if ((*it)->getSocket() == socket)
        {
            this->endpoints.erase(it);
            return ;
        }
    }
}

// Basic hooks called by server, redirect with correct type
void            GatewayInterface<Side::SERVER, Protocols::Transport::TCP>::onConnected(IServerClient* client)
{
    std::cout << "GatewayInterface<Side::SERVER, Protocols::Transport::TCP>::onConnected => reinterpreting back to client_type" << std::endl;
    this->onConnected(reinterpret_cast<client_type*>(client));
}

void            GatewayInterface<Side::SERVER, Protocols::Transport::TCP>::onDisconnected(IServerClient* client) 
{
    this->onDisconnected(reinterpret_cast<client_type*>(client));
}





/**********************************************************************/
/**                GATEWAY INTERFACE (CLIENT SIDE)                   **/
/**********************************************************************/

GatewayInterface<Side::CLIENT, Protocols::Transport::TCP>::GatewayInterface(Client& client)
: client(client), connections(), _protocol()
{}

IProtocolMethod<Side::CLIENT>*     GatewayInterface<Side::CLIENT, Protocols::Transport::TCP>::getProtocol()
{
    return (reinterpret_cast<IProtocolMethod<Side::CLIENT>*>(&this->_protocol));
}

void    GatewayInterface<Side::CLIENT, Protocols::Transport::TCP>::attachToConnection(IClientConnection* endpoint)
{
    connection_type* ep = dynamic_cast<connection_type*>(endpoint);
    if (ep == nullptr)
        return ;
    this->connections.push_back(ep);
}

void    GatewayInterface<Side::CLIENT, Protocols::Transport::TCP>::detachFromConnection(const int socket)
{
    for (std::vector<connection_type*>::iterator it = this->connections.begin(); it != this->connections.end(); ++it)
    {
        if ((*it)->getSocket() == socket)
        {
            this->connections.erase(it);
            return ;
        }
    }
}

// Basic hooks called by server, redirect with correct type
void    GatewayInterface<Side::CLIENT, Protocols::Transport::TCP>::onConnected(IClientConnection* connection)
{
    this->onConnected(reinterpret_cast<connection_type*>(connection));
}

void    GatewayInterface<Side::CLIENT, Protocols::Transport::TCP>::onDisconnected(IClientConnection* connection)
{
    this->onDisconnected(reinterpret_cast<connection_type*>(connection));
}






/**********************************************************************/
/**                  PROTOCOL METHOD (SERVER SIDE)                   **/
/**********************************************************************/

ProtocolMethod<Side::SERVER, Protocols::Transport::TCP>::ProtocolMethod()
: IProtocolMethod()
{}

int ProtocolMethod<Side::SERVER, Protocols::Transport::TCP>::generateSocket() const
{
    return (::socket(AF_INET, SOCK_STREAM, 0));
}

int ProtocolMethod<Side::SERVER, Protocols::Transport::TCP>::receiveMethod(IServerClient* client)
{
    std::cout << "receive method" << std::endl;

    uint8_t data_buffer[RECV_BLK_SIZE] = {0};
    ssize_t recv_bytes = recv(client->getSocket(), data_buffer, RECV_BLK_SIZE, MSG_DONTWAIT);
    if (recv_bytes < 0)
        return -1;
    else if (recv_bytes == 0)
        return 0;
    
    interface_type::client_type* complete_client = reinterpret_cast<interface_type::client_type*>(client);
    interface_type *interface = reinterpret_cast<interface_type*>(client->getEndpoint()->getInterface());
    interface->onReceived(complete_client, std::string(reinterpret_cast<char*>(data_buffer), recv_bytes));
    //onReceived(cliypent, data_buffer, recv_bytes);
    
    return recv_bytes;
}

int ProtocolMethod<Side::SERVER, Protocols::Transport::TCP>::sendMethod(IServerClient* client, const void *data, const size_t size)
{
    ssize_t sent_bytes = send(client->getSocket(), data, size, MSG_DONTWAIT);
    if (sent_bytes < 0)
        return -1;
    else if (static_cast<size_t>(sent_bytes) < size)
        return sent_bytes;
    
    return 0;
}

int ProtocolMethod<Side::SERVER, Protocols::Transport::TCP>::acceptMethod(const int endpoint_socket, void *addr_buffer, socklen_t* addr_len)
{
    // TODO: proper family specification
    int socket = ::accept(endpoint_socket, reinterpret_cast<sockaddr*>(addr_buffer), addr_len);
    if (socket < 0)
        return (-1);
    
    return (socket);
}


/**********************************************************************/
/**                  PROTOCOL METHOD (CLIENT SIDE)                   **/
/**********************************************************************/

ProtocolMethod<Side::CLIENT, Protocols::Transport::TCP>::ProtocolMethod()
: IProtocolMethod()
{}

int ProtocolMethod<Side::CLIENT, Protocols::Transport::TCP>::generateSocket() const
{
    return (::socket(AF_INET, SOCK_STREAM, 0));
}

#define RECV_BLK_SIZE 1024
int ProtocolMethod<Side::CLIENT, Protocols::Transport::TCP>::receiveMethod(IClientConnection* connection)
{
    std::cout << "receive method" << std::endl;

    uint8_t data_buffer[RECV_BLK_SIZE] = {0};
    ssize_t recv_bytes = recv(connection->getSocket(), data_buffer, RECV_BLK_SIZE, MSG_DONTWAIT);
    if (recv_bytes < 0)
        return -1;
    else if (recv_bytes == 0)
        return 0;

    interface_type::connection_type* complete_connection = reinterpret_cast<interface_type::connection_type*>(connection);
    interface_type *interface = reinterpret_cast<interface_type*>(connection->getInterface());
    interface->onReceived(complete_connection, std::string(reinterpret_cast<char*>(data_buffer), recv_bytes));            
    return recv_bytes;
}

int ProtocolMethod<Side::CLIENT, Protocols::Transport::TCP>::sendMethod(IClientConnection* client, const void *data, const size_t size)
{
    ssize_t sent_bytes = send(client->getSocket(), data, size, MSG_DONTWAIT);
    if (sent_bytes < 0)
        return -1;
    else if (static_cast<size_t>(sent_bytes) < size)
        return sent_bytes;
    
    return 0;
}










// ProtocolMethod<Protocol::TCP>::ProtocolMethod()
// : IProtocolMethod()
// {}

// int ProtocolMethod<Protocol::TCP>::generateSocket() const
// {
//     // TODO: proper family specification
//     return (::socket(AF_INET, SOCK_STREAM, 0));
// }
// #include <iostream>
// int ProtocolMethod<Protocol::TCP>::receiveMethod(const int socket, void *data, const size_t size)
// {
//     std::cout << "receive method" << std::endl;
//     ssize_t recv_bytes = recv(socket, data, size, MSG_DONTWAIT);
//     if (recv_bytes < 0)
//         return -1;
//     else if (recv_bytes == 0)
//         return 0;
    
//     return recv_bytes;
// }

// int ProtocolMethod<Protocol::TCP>::sendMethod(const int socket, const void *data, const size_t size)
// {
//     ssize_t sent_bytes = send(socket, data, size, MSG_DONTWAIT);
//     if (sent_bytes < 0)
//         return -1;
//     else if (static_cast<size_t>(sent_bytes) < size)
//         return sent_bytes;
    
//     return 0;
// }
// #include <iostream>
// int ProtocolMethod<Protocol::TCP>::acceptMethod(const int endpoint_socket, void* addr_buffer, socklen_t* addr_len)
// {
//     // TODO: proper family specification
//     int socket = ::accept(endpoint_socket, reinterpret_cast<sockaddr*>(addr_buffer), addr_len);
//     if (socket < 0)
//         return (-1);
    
//     return (socket);
// }