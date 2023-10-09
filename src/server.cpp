#include "server.hpp"


/* ================================================ */
/* Constructors                                     */
/* ================================================ */

// default constructor
Server::Server(void)
: endpoints(),
    running(false),
    n_clients_connected(0),
    max_connections(-1)
#ifdef ENABLE_TLS
    ,_ssl_method(TLS_server_method())
#endif
{
#ifdef ENABLE_TLS
    SSL_library_init();
    // this->_init_ssl_ctx();
#endif
}

/* Server constructor for multiple endpoints                               */
/* this call can throw if one of the endpoints contains an invalid address */
/* and thus needs to be catched.                                           */
/* If TLS is enabled, it can also throw a SSLInitException.                */
Server::Server(const std::list<ServerEndpoint>& endpoints)
: endpoints(endpoints),
    running(false),
    n_clients_connected(0),
    max_connections(-1)
#ifdef ENABLE_TLS
    ,_ssl_method(TLS_server_method())
#endif
{
#ifdef ENABLE_TLS
    SSL_library_init();
    // this->_init_ssl_ctx();
#endif
}



Server::~Server()
{
    this->shutdown();
    
#ifdef ENABLE_TLS
    SSL_CTX_free(this->_ssl_ctx);
#endif
}




/* ================================================ */
/* Public Members                                   */
/* ================================================ */


/* Makes the server starts listening,                                          */
/* after that, wait_update should be called to gather informations.            */
/* This call can throw an exception if any of the specified endpoints cannot   */
/* bind the specified address of that listen fails, when TLS is used, it loads */
/* the certificates and can thus throw a SSLCertException                      */
void    Server::start_listening(int max_pending_connections)
{
#ifdef ENABLE_TLS
    bool enablesTLS = false;
#endif
    for (ServerEndpoint& ep : this->endpoints)
    {
#ifdef ENABLE_TLS
        if (ep.useTLS())
            enablesTLS = true;
#endif
        ep.start_listening(max_pending_connections);
        // set endpoint to be selected 
        this->_poll_handler.addSocket(ep.getSocket());
    }

#ifdef ENABLE_TLS
    // if (enablesTLS)
    //     this->_load_ssl_certs();
#endif
    this->running = true;
}



/* Waits for an update on the read fds, needs to be called every time        */
/* It will gather informations received on sockets and run clients handlers  */
/* This function should be called after doing some operations on the clients */
/* externally so that the data processed has less latency                    */
/* It will also perform all the emits requested before its call              */
/* It returns true if the server is still active after completion            */
/* If specified, a timeout argument can be set to define the                 */
/* timeout in ms for select                                                  */
bool    Server::wait_update(const int timeout_ms) noexcept
{
    if (!running)
        return (false);
    
    if (this->_poll_handler.processPoll(timeout_ms))
    {
        if (errno == EINTR || errno == ENOMEM)
            return (true);
        //LOG_ERROR(LOG_CATEGORY_NETWORK, "sockets handler failed: " << strerror(errno));
        return (false);
    }

    // looking for each handeled events
    SocketsHandler::socket_event ev = this->_poll_handler.nextSocketEvent();
    while (!EV_IS_END(ev))
    {
        std::cout  << "sockevent" <<std::endl;
        // looking for new connections on endpoints                
        switch (this->_handleServerEvent(ev))
        {
            case 0:
                break ;
            // some event processed
            case 1:
                ev = this->_poll_handler.nextSocketEvent();
                continue;
            // an error occurred
            case 2:
                // terminate all if no endpoints remaining
                return this->endpoints.size() > 0;
        }

        std::cout  << "client sockevent" <<std::endl;

        // looking for clients received data
        if (!this->_handleClientsEvent(ev))
            return (running); 

        ev = this->_poll_handler.nextSocketEvent();
    }

    return (running);
}


// finds a client by searching all the endpoints
// this is quite slow 
ServerClient* Server::findClient(int socket)
{
    for (ServerEndpoint& ep: this->endpoints)
    {
        // std::cout << "check endpoint " << ep.getHostname() << ":" << ep.getPort() << std::endl;
        // since clients in ServerEndpoint are stored in a std::map
        // keyed by the client's socket, we can filter out ServerEndpoints
        // that can't contain the socket
        if (ep.getClients().begin()->first > socket
            || ep.getClients().end()->first < socket)
            continue ;
        
        try {
            return &ep.getClient(socket);
        }
        catch(const std::exception& e) {
            continue;
        }
    }
    return (nullptr);
}


void	Server::sendData(ServerClient& client, const void *data, size_t data_size)
{
    std::cout << "queued emit to " << client.getHostname() << std::endl;
    client.appendSendBuffer(reinterpret_cast<const uint8_t*>(data), data_size);
    this->_poll_handler.socketWantsWrite(client.getSocket(), true);
}


/* ================================================ */
/* Other Utils                                      */
/* ================================================ */

/* disconnects the specified client from the server*/
bool    Server::disconnect(ServerClient& client)
{
    int socket = client.getSocket();
    std::string address = client.getHostname();

#ifdef ENABLE_TLS
    if (client._useTLS && client._ssl_connection != nullptr)
    {
        SSL_free(client._ssl_connection);
        client._ssl_connection = nullptr;
    }
#endif

    client.getEndpoint().getInterface()->onDisconnected(client);
    this->_poll_handler.delSocket(client.getSocket());
    client.getEndpoint().delClient(socket);
    // TODO better check deletion result

    ::close(socket);
    this->n_clients_connected--;
    // LOG_INFO(LOG_CATEGORY_NETWORK, "client from " << address << " disconnected");
    return (true);
}

/* shutdowns the server an closes all connections */
void    Server::shutdown()
{
    for (ServerEndpoint& ep : this->endpoints)
        ep.close();//::close(ep.getSocket());
    this->running = false;
}



/* ================================================ */
/* Private members                                  */
/* ================================================ */

// for clients trying to connect on the endpoints
// returns:
// 0 -> no event processed, no error
// 1 -> one event processed, no error
// >1 -> error
int    Server::_handleServerEvent(const SocketsHandler::socket_event& ev)
{
    for (std::list<ServerEndpoint>::iterator ep = this->endpoints.begin(); ep != this->endpoints.end(); ++ep)
    {
        if (!EV_IS_SOCKET(ev, ep->getSocket()))
            continue ;
        if (EV_IS_ERROR(ev))
        {
            //LOG_ERROR(LOG_CATEGORY_NETWORK, "An error occurred on endpoint " << ep.getHostname());
            std::cout << "An error occurred on endpoint " << ep->getHostname() << std::endl;
            this->_poll_handler.delSocket(ep->getSocket());
            this->endpoints.erase(ep);
            return (2);
        }
        if (!EV_IS_READABLE(ev))
            continue ;
#ifdef ENABLE_TLS
        if (ep->useTLS())
                //TODO
            ;//this->_accept_ssl(*ep);
        else 
#endif
        if (ep->getAddressFamily() == AF_INET)
        {
            this->_accept(*ep);
        }
        else if (ep->getAddressFamily() == AF_INET6)
        {
            // TODO
            // this->_accept6(*ep);
        }
        return (1);
    }
    return (0);
}

// for the clients that are already connected to an endpoint
// returns:
// true: no errors occured
// false: an error occured, running needs to be checked in wait_update in case 
//        a client called shutdown()
bool    Server::_handleClientsEvent(const SocketsHandler::socket_event& ev)
{
    try {
        ServerClient* client = this->findClient(ev.socket);
        if (client == nullptr)
        {
            // std::cout << "No client found for Server::_handleClientsEvent" << std::endl;
            return (false);
        }
    
        if (EV_IS_ERROR(ev))
        {
            this->disconnect(*client);
            return (false);
        }
        if (EV_IS_READABLE(ev))
        {
            if (this->_receive(*client) == false)
            return (false);
        }
        if (EV_IS_WRITABLE(ev))
        {
            this->_send_data(*client);
        }
    } catch (std::out_of_range& e)
    {
        //LOG_ERROR(LOG_CATEGORY_NETWORK, "SocketsHandler returned an event with no recipient !!!");
        return (false);
    }
    return (true);
}


/* ================================================ */
/* Accept Handlers                                  */
/* ================================================ */

int     Server::_accept(ServerEndpoint& endpoint)
{
    sockaddr_in     client_addr;
    socklen_t       len = sizeof(client_addr);
    std::memset(&client_addr, 0, len);

    if (this->max_connections > 0 && this->n_clients_connected >= this->max_connections)
    {
        std::cout << "Cannot accept more than " << this->max_connections << " connections, refusing new client." << std::endl;
        // LOG_INFO(LOG_CATEGORY_NETWORK, "Cannot accept more than " << this->max_connections << " connections, refusing new client from "  << addr_info.getHostname());
        // cannot accept more clients
        // flushes the endpoint socket
        int client_socket = endpoint.getInterface()->getProtocol()->acceptMethod(endpoint.getSocket(), &client_addr, &len);
        close (client_socket);
        return (-1);
    }

    
    // int client_socket = ::accept(endpoint.getSocket(), (sockaddr*)&client_addr, &len);
    int client_socket = endpoint.getInterface()->getProtocol()->acceptMethod(endpoint.getSocket(), &client_addr, &len);
    if (client_socket <= 0)
    {
        std::cout << "Accept failed on endpoint " << endpoint.getHostname() << " with error: " << std::strerror(errno) << std::endl;
        // LOG_WARN(LOG_CATEGORY_NETWORK, "Accept failed on endpoint " << endpoint.getHostname() << " with error: " << std::strerror(errno));
        return (-1);
    }
    
    InetAddress     addr_info = InetAddress(client_addr);
    ServerClient*   client = endpoint.addClient(ServerClient(endpoint, client_socket, addr_info));
    if (client == nullptr)
    {
        std::cout << "Client insertion in std::map failed on endpoint " << endpoint.getHostname() << " for client just arrived from "  << addr_info.getHostname() << std::endl;
        // LOG_ERROR(LOG_CATEGORY_NETWORK, "Client insertion in std::map failed on endpoint " << endpoint.getHostname() << " for client just arrived from "  << addr_info.getHostname());
        ::close(client_socket);
        return (-1);
    }

    this->n_clients_connected++;
    this->_poll_handler.addSocket(client->getSocket());
    
    // LOG_INFO(LOG_CATEGORY_NETWORK, "New client connected on endpoint " << endpoint.getHostname() << " from "  << addr_info.getHostname());
    endpoint.getInterface()->onConnected(*client);
    return (client->getSocket());
}

// TODO REFRACTOR FOR NEW ENDPOINTS
// int     Server::_accept6(ServerEndpoint& endpoint)
// {
//     sockaddr_in6    client_addr;
//     socklen_t       len = sizeof(client_addr);
//     std::memset(&client_addr, 0, len);
//     int client_socket = ::accept(endpoint.getSocket(), (sockaddr*)&client_addr, &len);
//     if (client_socket <= 0)
//     {
//         LOG_WARN(LOG_CATEGORY_NETWORK, "Accept failed on IPv6 on endpoint " << endpoint.getHostname() << " with error: " << std::strerror(errno));
//         return (-1);
//     }
    
//     try {
//         InetAddress addr_info = InetAddress(client_addr);
//         if (this->max_connections > 0 && this->n_clients_connected >= this->max_connections)
//         {
//             LOG_INFO(LOG_CATEGORY_NETWORK, "Cannot accept more than " << this->max_connections << " connections, refusing new client from "  << addr_info.getHostname());
//             // cannot accept more clients
//             close (client_socket);
//             return (-1);
//         }
        
//         std::pair<typename Server::client_list_type::iterator, bool> insertion = this->_clients.insert(std::make_pair(client_socket, client_type(client_socket, addr_info)));
//         if (insertion.second == false)
//         {
//             LOG_ERROR(LOG_CATEGORY_NETWORK, "Client insertion in std::map failed on endpoint " << endpoint.getHostname() << " for client just arrived from "  << addr_info.getHostname());
//             ::close(client_socket);
//             return (-1);
//         }

//         this->n_clients_connected++;
//         this->_poll_handler.addSocket(insertion.first->second);
//         LOG_INFO(LOG_CATEGORY_NETWORK, "New client connected on IPv6 on endpoint " << endpoint.getHostname() << " from "  << addr_info.getHostname());
//         this->_client_handler.onConnected((*insertion.first).second);
//         return (client_socket);
//     }
//     catch (std::exception &e)
//     {
//         LOG_ERROR(LOG_CATEGORY_NETWORK, "Client creation failed for connection on IPv6: " << e.what());
//         ::close(client_socket);
//         return (-1);
//     }
// }

#ifdef ENABLE_TLS

// TODO REFRACTOR FOR NEW ENDPOINTS
// like in openssl:
// returns 1  => done
// returns 0  => expects more data
// returns -1 => invalid
// int     Server::_ssl_do_accept(ServerClient& client)
// {
//     // ssl protocol accept (blocks process, maybe set a timeout or handle with select)
//     int accept_error;
//     if ( (accept_error = SSL_accept(client.getSSL())) != 1 )
//     {
//         int ssl_error = SSL_get_error(client.getSSL(), accept_error);
//         if (ssl_error == SSL_ERROR_WANT_READ)
//         {
//             LOG_INFO(LOG_CATEGORY_NETWORK, "SSL accept incomplete, waiting for more data for handshake...");
//             return (0);
//         }
//         LOG_WARN(LOG_CATEGORY_NETWORK, "SSL accept handshake failed for client trying to connect from " << client.getHostname());
//         ERR_print_errors_fp(stderr);
//         return (-1);
//     }
//     client._accept_done = true;
//     LOG_INFO(LOG_CATEGORY_NETWORK, "Upgraded connection to TLS for client connected from " << client.getHostname());
//     //this->_client_handler.onConnected(client);
//     // TODO endpoint.interface.onConnected(client);
//     return (1);
// }

// int     Server::_accept_ssl(ServerEndpoint& endpoint)
// {
//     sockaddr_in     client_addr;
//     socklen_t       len = sizeof(client_addr);
//     std::memset(&client_addr, 0, len);
//     int client_socket = ::accept(endpoint.getSocket(), (sockaddr*)&client_addr, &len);
//     if (client_socket <= 0)
//     {
//         LOG_WARN(LOG_CATEGORY_NETWORK, "Accept failed on endpoint " << endpoint.getHostname() << " with error: " << std::strerror(errno));
//         return (-1);
//     }
    
//     try {
//         InetAddress addr_info = InetAddress(client_addr);
//         if (this->max_connections > 0 && this->n_clients_connected >= this->max_connections)
//         {
//             LOG_INFO(LOG_CATEGORY_NETWORK, "Cannot accept more than " << this->max_connections << " connections, refusing new client from "  << addr_info.getHostname());
//             // cannot accept more clients
//             ::close (client_socket);
//             return (-1);
//         }

//         SSL*    ssl = SSL_new(this->_ssl_ctx);
//         SSL_set_fd(ssl, client_socket);
//         SSL_set_accept_state(ssl);
        
//         std::pair<typename Server::client_list_type::iterator, bool> insertion = this->_clients.insert(std::make_pair(client_socket, client_type(client_socket, addr_info, ssl)));
//         if (insertion.second == false)
//         {
//             LOG_ERROR(LOG_CATEGORY_NETWORK, "Client insertion in std::map failed on endpoint " << endpoint.getHostname() << " for client just arrived from " << addr_info.getHostname());
//             ::close(client_socket);
//             return (-1);
//         }

//         this->n_clients_connected++;
//         this->_poll_handler.addSocket(insertion.first->second);
//         LOG_INFO(LOG_CATEGORY_NETWORK, "New client was accepted on TLS on endpoint " << endpoint.getHostname() << " from "  << addr_info.getHostname() << ", waiting for SSL handshake...");
//         return (client_socket);
//     }
//     catch (std::exception &e)
//     {
//         LOG_ERROR(LOG_CATEGORY_NETWORK, "Client creation failed for connection on TLS IPv4: " << e.what());
//         ::close(client_socket);
//         return (-1);
//     }
// }
#endif



/* ================================================ */
/* Recv handler & unpacker                          */
/* ================================================ */

// must at least be sizeof(packet_data_header) (or sizeof(size_t) + 32)
bool     Server::_receive(ServerClient& from)
{
//     uint8_t buffer[RECV_BLK_SIZE] = {0};
// #ifdef ENABLE_TLS
//     ssize_t size;
//     if (from._useTLS)
//     {
//         if (!from._accept_done)
//         {
//             // TODO
//             // if (this->_ssl_do_accept(from) == -1)
//             // {
//             //     this->disconnect(from);
//             //     return false;
//             // }
//             return (true);
//         }
//         size = SSL_read(from._ssl_connection, buffer, RECV_BLK_SIZE);
//     }
//     else
//         size = recv(from.getSocket(), buffer, RECV_BLK_SIZE, MSG_DONTWAIT);
// #else
//     ssize_t size = recv(from.getSocket(), buffer, RECV_BLK_SIZE, MSG_DONTWAIT);
// #endif
    uint8_t buffer[RECV_BLK_SIZE] = {0};
    ssize_t size = from.getEndpoint().getInterface()->getProtocol()->receiveMethod(from.getSocket(), buffer, RECV_BLK_SIZE);
    std::cout << "recv" << std::endl;
    if (size == 0)
    {
        this->disconnect(from);
        return (false);
    }
    else if (size < 0)
    {
        // an error has occured, disconnect the client
        // LOG_ERROR(LOG_CATEGORY_NETWORK, "Recv failed for client from " << from.getHostname() << ": " << std::strerror(errno) << ", disconnecting client for safety.");
        this->disconnect(from);
        return (false);
    }

    std::cout << "appendBuff" << std::endl;
    // append the received buffer
    from.appendRecvBuffer(buffer, size);

    // evaluate the whole buffer
    std::cout << "evalResult" << std::endl;
    GatewayInterfaceBase<Side::SERVER>* interface = from.getEndpoint().getInterface();
    if (interface == nullptr)
        return (false);
    
    PacketParserBase::EvalResult eval = interface->getParser()->eval(from.getRecvBuffer().c_str(), from.getRecvSize());
    switch (eval)
    {
        case PacketParserBase::EvalResult::COMPLETE:
            std::cout << "COMPLETE" << std::endl;
            // from.getRecvSize() Should be the size of the parsed packet, calculated by eval
            interface->receive(from, interface->getParser()->parse(from.getRecvBuffer().c_str(), from.getRecvSize()));
            std::cout << "clearBuff" << std::endl;
            from.clearRecvBuffer(from.getRecvSize());
            break; 
        case PacketParserBase::EvalResult::INCOMPLETE:
            // incomplete packet, more data expected to be received, for now, do nothing
            // maybe add a "packet timeout" to deal with incomplete packets tails that will never arrive
            break; 
        case PacketParserBase::EvalResult::INVALID:
            // packet is invalid, clear all data
            // this might also clear the start of the next packet if received early
            // this would make the next packet also invalid
            from.clearRecvBuffer();
            break; 
    }
    std::cout << "after recv" << std::endl;
    return (true);
}



// Sends the data queued for client, returns true if data was flushed entierly.
bool    Server::_send_data(ServerClient& client)
{
    std::cout << "send_data" << std::endl;
    if (client.getSendSize() == 0)
    {
        //LOG_ERROR(LOG_CATEGORY_NETWORK, "fd_set was set for sending for client from " << client.getHostname() << " however no data is provider to send.")
        this->_poll_handler.socketWantsWrite(client.getSocket(), false);
        return false;
    }
    //LOG_INFO(LOG_CATEGORY_NETWORK, "emitting to client from " << client.getHostname());
// #ifdef ENABLE_TLS
//     ssize_t sent_bytes;
//     if (client._useTLS)
//     {
//         if (!client._accept_done)
//         {
//             // LOG_WARN(LOG_CATEGORY_NETWORK, "Attempting to emit to TLS client from " << client.getHostname() << " which is not yet accepted, setting emit for later...");
//             return false;
//         }
//         sent_bytes = SSL_write(client._ssl_connection, client.getSendBuffer().c_str(), client.getSendSize());
//     }
//     else
//         sent_bytes = send(client.getSocket(), client.getSendBuffer().c_str(), client.getSendSize(), 0);
// #else
//     ssize_t sent_bytes = send(client.getSocket(), client.getSendBuffer().c_str(), client.getSendSize(), 0);
//     std::cout << "send" << std::endl;
// #endif
    ssize_t sent_bytes = client.getEndpoint().getInterface()->getProtocol()->sendMethod(client.getSocket(), client.getSendBuffer().c_str(), client.getSendSize());
    if (sent_bytes < 0)
    {
        // LOG_WARN(LOG_CATEGORY_NETWORK, "Send to client from " << client.getHostname() << " failed with error: " << std::strerror(errno))
        client.clearSendBuffer();
        if (client.getSendSize() == 0)
            this->_poll_handler.socketWantsWrite(client.getSocket(), false);
        return false;
    }
    else if (sent_bytes > 0)
    {
    //     // LOG_WARN(LOG_CATEGORY_NETWORK, "sent 0 bytes of data to client from " << client.getHostname());
    //     return false;
    // }
    // else if ((size_t)sent_bytes != client.getSendSize())
    // {
        client.clearSendBuffer(sent_bytes);
        // LOG_INFO(LOG_CATEGORY_NETWORK, "Data sent to client from " << client.getHostname()<< " was cropped: " << client._data_to_send.top().length() << " bytes left to send");
        return false;
    }
    // sent full packet.
    client.clearSendBuffer();
    if (client.getSendSize() == 0)
        this->_poll_handler.socketWantsWrite(client.getSocket(), false);
    return (true);
}
