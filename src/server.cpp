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
Server::Server(const std::list<IServerEndpoint*>& endpoints)
:   endpoints(),
    running(false),
    n_clients_connected(0),
    max_connections(-1)
#ifdef ENABLE_TLS
    ,_ssl_method(TLS_server_method())
#endif
{
    for (IServerEndpoint* ep : endpoints)
    {
        ep->start_listening(10);
        this->endpoints.insert(std::make_pair(ep->getSocket(), ep));
        this->_poll_handler.addSocket(ep->getSocket());
    }
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
    (void)max_pending_connections;
// #ifdef ENABLE_TLS
//     bool enablesTLS = false;
// #endif
//     for (ServerEndpoint& ep : this->endpoints)
//     {
// #ifdef ENABLE_TLS
//         if (ep.useTLS())
//             enablesTLS = true;
// #endif
//         ep.start_listening(max_pending_connections);
//         // set endpoint to be selected 
//         this->_poll_handler.addSocket(ep.getSocket());
//     }

// #ifdef ENABLE_TLS
//     // if (enablesTLS)
//     //     this->_load_ssl_certs();
// #endif
//     this->running = true;
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
    {
        std::cout << "wait_update() called on a non-running server" << std::endl;
        return (false);
    }
    
    std::cout << "polling" << std::endl;
    if (this->_poll_handler.processPoll(timeout_ms))
    {
        if (errno == EINTR || errno == ENOMEM)
            return (true);
        std::cout << "sockets handler failed: " << strerror(errno) << std::endl;
        //LOG_ERROR(LOG_CATEGORY_NETWORK, "sockets handler failed: " << strerror(errno));
        return (false);
    }

    // looking for each handeled events
    std::cout << "parsing events" << std::endl;
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
IServerClient* Server::findClient(int socket)
{
    for (std::map<int, IServerEndpoint*>::iterator ep = this->endpoints.begin(); ep != this->endpoints.end(); ++ep)
    {
        std::cout << "searching client on EndPoint " << ep->second->getIpAddress() << ":"<< ep->second->getPort() << " (*" << /**ep->second->getClients().size() <<*/ " connected)" << std::endl;
        // if (ep->second->getClients().empty())
        //     continue ;
        
        // since clients in ServerEndpoint are stored in a std::map
        // keyed by the client's socket, we can filter out ServerEndpoints
        // that can't contain the socket
        // if (ep.getClients().begin()->first < socket || ep.getClients().end()->first > socket)
        //     continue;
        
        try {
            IServerClient* client = ep->second->getClient(socket);
            if (client != nullptr)
                return (client);
        }
        catch(const std::exception& e) {
            continue;
        }
    }
    return (nullptr);
}


void	Server::sendData(IServerClient* client, const void *data, size_t data_size)
{
    std::cout << "queued emit to " << client->getHostname() << std::endl;
    client->appendSendBuffer(reinterpret_cast<const uint8_t*>(data), data_size);
    this->_poll_handler.socketWantsWrite(client->getSocket(), true);
}


/* ================================================ */
/* Other Utils                                      */
/* ================================================ */

/* disconnects the specified client from the server*/
bool    Server::disconnect(IServerClient* client)
{
    int socket = client->getSocket();
    std::string address = client->getHostname();

#ifdef ENABLE_TLS
    if (client->_useTLS && client->_ssl_connection != nullptr)
    {
        SSL_free(client->_ssl_connection);
        client->_ssl_connection = nullptr;
    }
#endif

    client->getEndpoint()->getInterface()->onDisconnected(client);
    this->_poll_handler.delSocket(client->getSocket());
    client->getEndpoint()->delClient(socket);
    // TODO better check deletion result

    ::close(socket);
    this->n_clients_connected--;
    // LOG_INFO(LOG_CATEGORY_NETWORK, "client from " << address << " disconnected");
    return (true);
}

/* shutdowns the server an closes all connections */
void    Server::shutdown()
{
   // for (ServerEndpoint& ep : this->endpoints)
    for (std::map<int, IServerEndpoint*>::iterator ep = this->endpoints.begin(); ep != this->endpoints.end(); ++ep)
        ep->second->close();//::close(ep.getSocket());
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
    std::map<int, IServerEndpoint*>::iterator it = this->endpoints.find(ev.socket);
    if (it == this->endpoints.end())
        return (0);
    IServerEndpoint* ep = it->second;

    // for (std::list<ServerEndpoint>::iterator ep = this->endpoints.begin(); ep != this->endpoints.end(); ++ep)
    // {
        // if (!EV_IS_SOCKET(ev, ep->getSocket()))
        //     continue ;
    if (EV_IS_ERROR(ev))
    {
        std::cout << "EVENT ERROR: Server::_handleServerEvent" << std::endl;
        //LOG_ERROR(LOG_CATEGORY_NETWORK, "An error occurred on endpoint " << ep.getHostname());
        std::cout << "An error occurred on endpoint " << ep->getHostname() << std::endl;
        this->_poll_handler.delSocket(ep->getSocket());
        this->endpoints.erase(ep->getSocket());
        return (2);
    }
    if (!EV_IS_READABLE(ev))
        return (0);
#ifdef ENABLE_TLS
    if (ep->useTLS())
            //TODO
        ;//this->_accept_ssl(*ep);
    else 
#endif
    if (ep->getAddressFamily() == AF_INET)
    {
        if (this->_accept(ep) < 0)
            return (2);
    }
    else if (ep->getAddressFamily() == AF_INET6)
    {
        // TODO
        // this->_accept6(*ep);
    }
        // return (1);
    // }
    return (1);
}

// for the clients that are already connected to an endpoint
// returns:
// true: no errors occured
// false: an error occured, running needs to be checked in wait_update in case 
//        a client called shutdown()
bool    Server::_handleClientsEvent(const SocketsHandler::socket_event& ev)
{
    try {
        IServerClient* client = this->findClient(ev.socket);
        if (client == nullptr)
        {
            std::cout << "client not found for socket " << ev.socket << std::endl;
            // std::cout << "No client found for Server::_handleClientsEvent" << std::endl;
            return (false);
        }
    
        if (EV_IS_ERROR(ev))
        {
            std::cout << "EVENT ERROR: Server::_handleClientsEvent" << std::endl;
            this->disconnect(client);
            return (false);
        }
        if (EV_IS_READABLE(ev))
        {
            if (this->_receive(client) == false)
            return (false);
        }
        if (EV_IS_WRITABLE(ev))
        {
            this->_send_data(client);
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

int     Server::_accept(IServerEndpoint* endpoint)
{
    std::cout << "Server::_accept" << std::endl;

    sockaddr_in     client_addr;
    socklen_t       len = sizeof(client_addr);
    std::memset(&client_addr, 0, len);

    if (this->max_connections > 0 && this->n_clients_connected >= this->max_connections)
    {
        std::cout << "Cannot accept more than " << this->max_connections << " connections, refusing new client." << std::endl;
        // cannot accept more clients
        // flushes the endpoint socket
        int client_socket = endpoint->getInterface()->getProtocol()->acceptMethod(endpoint->getSocket(), &client_addr, &len);
        close (client_socket);
        return (-1);
    }

    // 1. Accept using interfaces protocol method
    std::cout << "ProtocolMethod::acceptMethod()" << std::endl;
    int client_socket = endpoint->getInterface()->getProtocol()->acceptMethod(endpoint->getSocket(), &client_addr, &len);
    if (client_socket <= 0)
    {
        std::cout << "Accept failed on endpoint " << endpoint->getHostname() << " with error: " << std::strerror(errno) << std::endl;
        return (-1);
    }
    
    std::cout << "accept success, creating ServerClient instance" << std::endl;
    // 2. get inet address informations of client and create a ServerClient
    InetAddress     addr_info = InetAddress(client_addr);
    
    // const decltype(endpoint->_interface) i = nullptr;

    // get data type from interface
    
    //(decltype(*endpoint->getInterface()))::client_data_type a;
    IServerClient* client = endpoint->createClient(client_socket, addr_info);
    //endpoint->addClient(ServerClient(endpoint, client_socket, addr_info));
    if (client == nullptr)
    {
        std::cout << "Client insertion in std::map failed on endpoint " << endpoint->getHostname() << " for client just arrived from "  << addr_info.getHostname() << std::endl;
        ::close(client_socket);
        return (-1);
    }

    std::cout << "client inserted successfully" << std::endl;

    this->n_clients_connected++;

    std::cout << "adding socket to poll handler" << std::endl;
    this->_poll_handler.addSocket(client->getSocket());
    
    std::cout << "calling onConnected()" << std::endl;
    endpoint->getInterface()->onConnected(client);

    std::cout << "ACCEPT_DONE" << std::endl;
    return (client->getSocket());
}


/* ================================================ */
/* Recv handler & unpacker                          */
/* ================================================ */

// must at least be sizeof(packet_data_header) (or sizeof(size_t) + 32)
bool     Server::_receive(IServerClient* from)
{
    std::cout << "Server::_receive" << std::endl;
    // uint8_t buffer[RECV_BLK_SIZE] = {0};
    ssize_t size = from->getEndpoint()->getInterface()->getProtocol()->receiveMethod(from);
    if (size == 0)
    {
        std::cout << "recv read 0" << std::endl;
        this->disconnect(from);
        return (false);
    }
    else if (size < 0)
    {
        std::cout << "recv error: " << strerror(errno) << std::endl;
        // an error has occured, disconnect the client
        // LOG_ERROR(LOG_CATEGORY_NETWORK, "Recv failed for client from " << from.getHostname() << ": " << std::strerror(errno) << ", disconnecting client for safety.");
        this->disconnect(from);
        return (false);
    }

    /*** NOW PROTOCOL DEPENDENT (DEFINED IN IProtocolMethod<Protocols::Application::MESSAGES>::receiveMethod) ****/
    // std::cout << "appending received data to buffer" << std::endl;
    // // append the received buffer
    // from->appendRecvBuffer(buffer, size);

    // // evaluate the whole buffer
    // std::cout << "evaluating packet: ";
    // IGatewayInterface<Side::SERVER>* interface = from->getEndpoint()->getInterface();
    // if (interface == nullptr)
    //     return (false);
    
    // IPacketParser<Side::SERVER>::EvalResult eval = interface->getParser()->eval(from.getRecvBuffer().c_str(), from.getRecvSize());
    // switch (eval)
    // {
    //     case IPacketParser::EvalResult::COMPLETE:
    //         std::cout << "COMPLETE" << std::endl;
    //         // from.getRecvSize() Should be the size of the parsed packet, calculated by eval
    //         interface->receive(from, interface->getParser()->parse(from.getRecvBuffer().c_str(), from.getRecvSize()));
    //         std::cout << "clearing receive buffer" << std::endl;
    //         from.clearRecvBuffer(from.getRecvSize());
    //         break; 
    //     case IPacketParser::EvalResult::INCOMPLETE:
    //         std::cout << "INCOMPLETE" << std::endl;
    //         // incomplete packet, more data expected to be received, for now, do nothing
    //         // maybe add a "packet timeout" to deal with incomplete packets tails that will never arrive
    //         break; 
    //     case IPacketParser::EvalResult::INVALID:
    //         std::cout << "INVALID" << std::endl;
    //         // packet is invalid, clear all data
    //         // this might also clear the start of the next packet if received early
    //         // this would make the next packet also invalid
    //         std::cout << "clearing receive buffer" << std::endl;
    //         from.clearRecvBuffer();
    //         break; 
    // }
    return (true);
}



// Sends the data queued for client, returns true if data was flushed entierly.
bool    Server::_send_data(IServerClient* client)
{
    std::cout << "Server::_send_data" << std::endl;
    if (client->getSendSize() == 0)
    {
        std::cout << "fd_set was set for sending for client from " << client->getHostname() << " however no data is provider to send." << std::endl;
        this->_poll_handler.socketWantsWrite(client->getSocket(), false);
        return false;
    }
 
    ssize_t sent_bytes = client->getEndpoint()->getInterface()->getProtocol()->sendMethod(client, client->getSendBuffer().c_str(), client->getSendSize());
    if (sent_bytes < 0)
    {
        std::cout << "Send to client from " << client->getHostname() << " failed with error: " << std::strerror(errno) << std::endl;
        client->clearSendBuffer();
        if (client->getSendSize() == 0)
            this->_poll_handler.socketWantsWrite(client->getSocket(), false);
        return false;
    }
    else if (sent_bytes > 0)
    {
        client->clearSendBuffer(sent_bytes);
        std::cout << "Data sent to client from " << client->getHostname()<< " was cropped: " << client->getSendBuffer() << " bytes left to send" << std::endl;
        return false;
    }
    // sent full packet.
    client->clearSendBuffer();
    if (client->getSendSize() == 0)
        this->_poll_handler.socketWantsWrite(client->getSocket(), false);
    return (true);
}
