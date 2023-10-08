#include "client.hpp"


/* ================================================ */
/* Constructors                                     */
/* ================================================ */

// default constructor
Client::Client(void)
:   connections()
#ifdef ENABLE_TLS
    ,_ssl_method(TLS_client_method())
#endif
{
#ifdef ENABLE_TLS
    SSL_library_init();
    // this->_init_ssl_ctx();
#endif
}

/* Client constructor for multiple connections                               */
/* this call can throw if one of the connections contains an invalid address */
/* and thus needs to be catched.                                           */
/* If TLS is enabled, it can also throw a SSLInitException.                */
Client::Client(const std::list<ClientConnection>& connections)
: connections(connections)
#ifdef ENABLE_TLS
    ,_ssl_method(TLS_client_method())
#endif
{
#ifdef ENABLE_TLS
    SSL_library_init();
    // this->_init_ssl_ctx();
#endif
}



Client::~Client()
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
/* This call can throw an exception if any of the specified connections cannot   */
/* bind the specified address of that listen fails, when TLS is used, it loads */
/* the certificates and can thus throw a SSLCertException                      */
// void    Client::start_listening(int max_pending_connections)
// {
// #ifdef ENABLE_TLS
//     bool enablesTLS = false;
// #endif
//     for (ClientConnection& ep : this->connections)
//     {
// #ifdef ENABLE_TLS
//         if (ep.useTLS())
//             enablesTLS = true;
// #endif
//         ep.start_listening(max_pending_connections);
//         // set connection to be selected 
//         this->_poll_handler.addSocket(ep.getSocket());
//     }

// #ifdef ENABLE_TLS
//     // if (enablesTLS)
//     //     this->_load_ssl_certs();
// #endif
//     this->running = true;
// }



/* Waits for an update on the read fds, needs to be called every time        */
/* It will gather informations received on sockets and run clients handlers  */
/* This function should be called after doing some operations on the clients */
/* externally so that the data processed has less latency                    */
/* It will also perform all the emits requested before its call              */
/* It returns true if the server is still active after completion            */
/* If specified, a timeout argument can be set to define the                 */
/* timeout in ms for select                                                  */
bool    Client::wait_update(const int timeout_ms) noexcept
{   
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
        // looking for clients received data
        if (!this->_handleClientEvent(ev))
            return (this->connections.size() > 0); 

        ev = this->_poll_handler.nextSocketEvent();
    }

    return (this->connections.size() > 0);
}


// finds a client by searching all the connections
ClientConnection* Client::findConnection(int socket)
{
    for (ClientConnection& c: this->connections)
    {
        if (c.getSocket() == socket)
            return (&c);
    }
    return (nullptr);
}

void    Client::sendData(ClientConnection& connection, const void* data, size_t data_size)
{
    std::cout << "queued emit to " << connection.getHostname() << std::endl;
    connection.appendSendBuffer(reinterpret_cast<const uint8_t*>(data), data_size);
    this->_poll_handler.socketWantsWrite(connection.getSocket(), true);
}

/* ================================================ */
/* Other Utils                                      */
/* ================================================ */

/* disconnects the specified client from the server*/
void    Client::closeConnection(ClientConnection& connection)
{
    int socket = connection.getSocket();
    std::string address = connection.getHostname();

#ifdef ENABLE_TLS
    if (connection._useTLS && connection._ssl_connection != nullptr)
    {
        SSL_free(connection._ssl_connection);
        connection._ssl_connection = nullptr;
    }
#endif

    connection.getInterface()->onDisconnected();
    this->_poll_handler.delSocket(connection.getSocket());
    connection.close();

    ::close(socket);
    // LOG_INFO(LOG_CATEGORY_NETWORK, "connection to " << address << " closed");
}

/* shutdowns the server an closes all connections */
void    Client::shutdown()
{
    for (ClientConnection& connection : this->connections)
        connection.close();
}



/* ================================================ */
/* Private members                                  */
/* ================================================ */

// for clients trying to connect on the connections
// returns:
// 0 -> no event processed, no error
// 1 -> one event processed, no error
// >1 -> error
// int    Client::_handleClientEvent(const SocketsHandler::socket_event& ev)
// {
//     for (std::list<ClientConnection>::iterator ep = this->connections.begin(); ep != this->connections.end(); ++ep)
//     {
//         if (!EV_IS_SOCKET(ev, ep->getSocket()))
//             continue ;
//         if (EV_IS_ERROR(ev))
//         {
//             //LOG_ERROR(LOG_CATEGORY_NETWORK, "An error occurred on connection " << ep.getHostname());
//             std::cout << "An error occurred on connection " << ep->getHostname() << std::endl;
//             this->_poll_handler.delSocket(ep->getSocket());
//             this->connections.erase(ep);
//             return (2);
//         }
//         if (!EV_IS_READABLE(ev))
//             continue ;
// #ifdef ENABLE_TLS
//         if (ep->useTLS())
//                 //TODO
//             ;//this->_accept_ssl(*ep);
//         else 
// #endif
//         if (ep->getAddressFamily() == AF_INET)
//         {
//             this->_accept(*ep);
//         }
//         else if (ep->getAddressFamily() == AF_INET6)
//         {
//             // TODO
//             // this->_accept6(*ep);
//         }
//         return (1);
//     }
//     return (0);
// }

// for the clients that are already connected to an connection
// returns:
// true: no errors occured
// false: an error occured, running needs to be checked in wait_update in case 
//        a client called shutdown()
bool    Client::_handleClientEvent(const SocketsHandler::socket_event& ev)
{
    // std::cout << "handle_event" << std::endl;
    try {
        // TODO move the clients list into the ClientConnection class
        ClientConnection* connection = this->findConnection(ev.socket);
        if (connection == nullptr)
        {
            // std::cout << "No connection found for Client::_handleClientsEvent" << std::endl;
            return (false);
        }
    
        if (EV_IS_ERROR(ev))
        {
            this->closeConnection(*connection);
            return (false);
        }
        if (EV_IS_READABLE(ev))
        {
            if (this->_receive(*connection) == false)
            return (false);
        }
        if (EV_IS_WRITABLE(ev))
        {
            this->_send_data(*connection);
        }
    } catch (std::out_of_range& e)
    {
        //LOG_ERROR(LOG_CATEGORY_NETWORK, "SocketsHandler returned an event with no recipient !!!");
        return (false);
    }
    return (true);
}


/* ================================================ */
/* Connect Handlers                                 */
/* ================================================ */

// bool     Client::_connect(ClientConnection& connection)
// {
  
// }

// #ifdef ENABLE_TLS

// bool     Client::_connect_ssl(ClientConnection& connection)
// {
  
// }
// #endif



/* ================================================ */
/* Recv handler & unpacker                          */
/* ================================================ */

// must at least be sizeof(packet_data_header) (or sizeof(size_t) + 32)
bool     Client::_receive(ClientConnection& connection)
{
    uint8_t buffer[RECV_BLK_SIZE] = {0};
#ifdef ENABLE_TLS
    ssize_t size;
    if (connection._useTLS)
    {
        if (!connection._accept_done)
        {
            // TODO
            // if (this->_ssl_do_accept(connection) == -1)
            // {
            //     this->disconnect(connection);
            //     return false;
            // }
            return (true);
        }
        size = SSL_read(connection._ssl_connection, buffer, RECV_BLK_SIZE);
    }
    else
        size = recv(connection.getSocket(), buffer, RECV_BLK_SIZE, MSG_DONTWAIT);
#else
    ssize_t size = recv(connection.getSocket(), buffer, RECV_BLK_SIZE, MSG_DONTWAIT);
#endif
    std::cout << "recv" << std::endl;
    if (size == 0)
    {
        this->closeConnection(connection);
        return (false);
    }
    else if (size < 0)
    {
        // we dont know what made recv fail, but for safety disconnect client.
        // LOG_ERROR(LOG_CATEGORY_NETWORK, "Recv failed for client connection " << connection.getHostname() << ": " << std::strerror(errno) << ", disconnecting client for safety.");
        this->closeConnection(connection);
        return (false);
    }

    std::cout << "appendBuff" << std::endl;
    // append the received buffer
    connection.appendRecvBuffer(buffer, size);

    // evaluate the whole buffer
    std::cout << "evalResult" << std::endl;
    ProtocolParserBase::EvalResult eval = connection.getInterface()->getParser()->eval(connection.getRecvBuffer().c_str(), connection.getRecvSize());
    switch (eval)
    {
        case ProtocolParserBase::EvalResult::COMPLETE:
            std::cout << "COMPLETE" << std::endl;
            // connection.getRecvSize() Should be the size of the parsed packet, calculated by eval
            connection.getInterface()->receive(connection.getInterface()->getParser()->parse(connection.getRecvBuffer().c_str(), connection.getRecvSize()));
            std::cout << "clearBuff" << std::endl;
            connection.clearRecvBuffer(connection.getRecvSize());
            break; 
        case ProtocolParserBase::EvalResult::INCOMPLETE:
            // incomplete packet, more data expected to be received, for now, do nothing
            // maybe add a "packet timeout" to deal with incomplete packets tails that will never arrive
            break; 
        case ProtocolParserBase::EvalResult::INVALID:
            // packet is invalid, clear all data
            // this might also clear the start of the next packet if received early
            // this would make the next packet also invalid
            connection.clearRecvBuffer();
            break; 
    }
    std::cout << "after recv" << std::endl;
    return (true);
}



// Sends the data queued for client, returns true if data was flushed entierly.
bool    Client::_send_data(ClientConnection& connection)
{
    std::cout << "send_data" << std::endl;
    if (connection.getSendSize() == 0)
    {
        //LOG_ERROR(LOG_CATEGORY_NETWORK, "fd_set was set for sending for connection from " << connection.getHostname() << " however no data is provider to send.")
        this->_poll_handler.socketWantsWrite(connection.getSocket(), false);
        return false;
    }
    //LOG_INFO(LOG_CATEGORY_NETWORK, "emitting to connection from " << connection.getHostname());
#ifdef ENABLE_TLS
    ssize_t sent_bytes;
    if (connection._useTLS)
    {
        if (!connection._accept_done)
        {
            // LOG_WARN(LOG_CATEGORY_NETWORK, "Attempting to emit to TLS connection from " << connection.getHostname() << " which is not yet accepted, setting emit for later...");
            return false;
        }
        sent_bytes = SSL_write(connection._ssl_connection, connection.getSendBuffer().c_str(), connection.getSendSize());
    }
    else
        sent_bytes = send(connection.getSocket(), connection.getSendBuffer().c_str(), connection.getSendSize(), 0);
#else
    ssize_t sent_bytes = send(connection.getSocket(), connection.getSendBuffer().c_str(), connection.getSendSize(), 0);
    std::cout << "send" << std::endl;
#endif
    if (sent_bytes < 0)
    {
        // LOG_WARN(LOG_CATEGORY_NETWORK, "Send to connection from " << connection.getHostname() << " failed with error: " << std::strerror(errno))
        connection.clearSendBuffer();
        if (connection.getSendSize() == 0)
            this->_poll_handler.socketWantsWrite(connection.getSocket(), false);
        return false;
    }
    else if (sent_bytes == 0)
    {
        // LOG_WARN(LOG_CATEGORY_NETWORK, "sent 0 bytes of data to connection from " << connection.getHostname());
        return false;
    }
    else if ((size_t)sent_bytes != connection.getSendSize())
    {
        connection.clearSendBuffer(sent_bytes); 
        // LOG_INFO(LOG_CATEGORY_NETWORK, "Data sent to connection from " << connection.getHostname()<< " was cropped: " << connection._data_to_send.top().length() << " bytes left to send");
        return false;
    }
    // sent full packet.
    connection.clearSendBuffer();
    if (connection.getSendSize() == 0)
        this->_poll_handler.socketWantsWrite(connection.getSocket(), false);
    return (true);
}
