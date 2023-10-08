#pragma once


#ifdef ENABLE_TLS
#include <openssl/ssl.h>
#include <openssl/err.h>
#endif

#include <cstdlib>
#include <cstring>
#include <string>
#include <iostream>
#include <map>
#include <stack>
#include <list>

/* SocketsHandler implementation is specified for each handler type */
#if   defined(USE_SELECT)
# ifndef DISABLE_WARNINGS
#  warning "select() Socket Handler is deprecated: use poll/kqueue on BSD plateforms, and epoll for Linux"
# endif
# include "common/socket_handlers/select_sockethandler.hpp"
#elif defined(USE_KQUEUE)
class SocketsHandler {};
# error "Socket Handler not implemented for kqueue()"
#elif defined(USE_EPOLL)
class SocketsHandler {};
# error "Socket Handler not implemented for epoll()"
#else // use POLL by default
# ifndef  USE_POLL
#  define USE_POLL
# endif
# include "common/socket_handlers/poll_sockethandler.hpp"
#endif


#include "protocols/raw/protocol_interface.hpp"
#include "protocols/raw/protocol_parser.hpp"

#include "server/server_client.hpp"
#include "server/server_endpoint.hpp"

class Server
{
    public:
        /* ================================================ */
        /* Constructors                                     */
        /* ================================================ */

        // default constructor
        Server(void);

        /* Server constructor for multiple endpoints                               */
        /* this call can throw if one of the endpoints contains an invalid address */
        /* and thus needs to be catched.                                           */
        /* If TLS is enabled, it can also throw a SSLInitException.                */
        Server(const std::list<ServerEndpoint>& endpoints);

        /* Server constructor for single endpoint                                  */
        /* this call can throw if the endpoint contains an invalid address.        */
        /* If TLS is enabled, it can also throw a SSLInitException.                */


/* Server constructor for single endpoint                                  */
/* this call can throw if the endpoint contains an invalid address.        */
/* If TLS is enabled, it can also throw a SSLInitException.                */
        template<class I>
#ifdef ENABLE_TLS
        Server(
			typename std::enable_if<std::is_base_of<GatewayInterfaceBase<Side::SERVER>, I>::value, const std::string&>::type
			ip_address,
			const int port,
			const bool useTLS,
			const sa_family_t family)
        : Server()
        {
        	this->addEndpoint<I>(ip_address, port, useTLS, family);
        }
#else
        Server(
			typename std::enable_if<std::is_base_of<GatewayInterfaceBase<Side::SERVER>, I>::value, const std::string&>::type
			ip_address,
			const int port,
			const sa_family_t family)
        : Server()
        {
            this->addEndpoint<I>(ip_address, port, family);
        }
#endif
        
        Server(const Server& copy) = delete;

        Server& operator=(const Server& other) = delete;

        ~Server();


        /* ================================================ */
        /* Public Members                                   */
        /* ================================================ */

        //template<Protocol P, GatewayInterface<Side::SERVER, P> I>
        template<class I>
        typename std::enable_if<std::is_base_of<GatewayInterfaceBase<Side::SERVER>, I>::value, void>::type
        addEndpoint(
            const std::string& ip_address,
            const int port,
            const sa_family_t family = AF_INET
        )
        {
            ServerEndpoint endpoint = ServerEndpoint(ip_address, port, family);
			this->endpoints.emplace_back(endpoint);
            GatewayInterfaceBase<Side::SERVER>* interface = new I(*this, *this->endpoints.rbegin());
            this->endpoints.rbegin()->setInterface(interface);
        }

        /* Makes the server starts listening,                                          */
        /* after that, wait_update should be called to gather informations.            */
        /* This call can throw an exception if any of the specified endpoints cannot   */
        /* bind the specified address of that listen fails, when TLS is used, it loads */
        /* the certificates and can thus throw a SSLCertException                      */
        void    start_listening(int max_pending_connections = 10);



        /* Waits for an update on the read fds, needs to be called every time        */
        /* It will gather informations received on sockets and run clients handlers  */
        /* This function should be called after doing some operations on the clients */
        /* externally so that the data processed has less latency                    */
        /* It will also perform all the emits requested before its call              */
        /* It returns true if the server is still active after completion            */
        /* If specified, a timeout argument can be set to define the                 */
        /* timeout in ms for select                                                  */
        bool    wait_update(const int timeout_ms = -1) noexcept;
       

        // finds a client by searching all the endpoints
        // this is quite slow 
        ServerClient* findClient(int socket);
        

    /* ================================================ */
    /* Private members                                  */
    /* ================================================ */
    private:
        // for clients trying to connect on the endpoints
        // returns:
        // 0 -> no event processed, no error
        // 1 -> one event processed, no error
        // >1 -> error
        int     _handleServerEvent(const SocketsHandler::socket_event& ev);
    
        // for the clients that are already connected to an endpoint
        // returns:
        // true: no errors occured
        // false: an error occured, running needs to be checked in wait_update in case 
        //        a client called shutdown()
        bool    _handleClientsEvent(const SocketsHandler::socket_event& ev);


        /* ================================================ */
        /* Accept Handlers                                  */
        /* ================================================ */

        int     _accept(ServerEndpoint& endpoint);
        

        // TODO REFRACTOR FOR NEW ENDPOINTS
        // int     _accept6(ServerEndpoint& endpoint);
        
#ifdef ENABLE_TLS

        // TODO REFRACTOR FOR NEW ENDPOINTS
        // like in openssl:
        // returns 1  => done
        // returns 0  => expects more data
        // returns -1 => invalid
        // int     _ssl_do_accept(ServerClient& client);

        // int     _accept_ssl(ServerEndpoint& endpoint);
#endif



        /* ================================================ */
        /* Recv handler & unpacker                          */
        /* ================================================ */

// must at least be sizeof(packet_data_header) (or sizeof(size_t) + 32)
#define RECV_BLK_SIZE   1024
        bool     _receive(ServerClient& from);

        // Sends the data queued for client, returns true if data was flushed entierly.
        bool    _send_data(ServerClient& client);


        /* ================================================ */
        /* Other Utils                                      */
        /* ================================================ */

        /* disconnects the specified client from the server*/
        bool    disconnect(ServerClient& client);

        /* shutdowns the server an closes all connections */
        void    shutdown();


    /* ================================================ */
    /* Public attributes                                */
    /* ================================================ */

    public:
        std::list<ServerEndpoint>   endpoints;

        bool        running;
        int         n_clients_connected;
        int         max_connections;

#ifdef ENABLE_TLS
        std::string ssl_cert_file;
        std::string ssl_private_key_file;
#endif

    /* ================================================ */
    /* Private attributes                               */
    /* ================================================ */

    private:
        SocketsHandler  _poll_handler;

#ifdef ENABLE_TLS
        SSL_CTX*            _ssl_ctx;
        const SSL_METHOD*   _ssl_method;
#endif
};