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


#include "server/server_client.hpp"
#include "server/server_endpoint.hpp"

class Server
{
    public:
	/* ================================================ */
	/* Constructors                                     */
	/* ================================================ */

	/* Server default constructor, endpoints should be added with addEndpoint  */
	Server(void);

	/* Server constructor for multiple endpoints                               */
	/* this call can throw if one of the endpoints contains an invalid address */
	/* and thus needs to be catched.                                           */
	/* If TLS is enabled, it can also throw a SSLInitException.                */
	Server(const std::list<IServerEndpoint*>& endpoints);

	/* Server constructor for single endpoint                                  */
	/* this call can throw if the endpoint contains an invalid address.        */
	/* If TLS is enabled, it can also throw a SSLInitException.                */
	template<class I>
#ifdef ENABLE_TLS
	Server(
			typename std::enable_if<std::is_base_of<IGatewayInterface<Side::SERVER>, I>::value, const std::string&>::type
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
			typename std::enable_if<std::is_base_of<IGatewayInterface<Side::SERVER>, I>::value, const std::string&>::type
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

		/* Adds a new ServerEndpoint with the provided GatewayInterface                                                                */
	template<class I>
	typename std::enable_if<std::is_base_of<IGatewayInterface<Side::SERVER>, I>::value, void>::type
	addEndpoint(
	    const std::string& ip_address,
	    const int port,
	    const sa_family_t family = AF_INET
	)
	{
	    // create the interface
	    //IGatewayInterface<Side::SERVER>* interface = new I(*this);
		I* interface = new I(*this);

		// create an endpoint for this interface
		ServerEndpoint<typename I::protocol_type> *endpoint = new ServerEndpoint<typename I::protocol_type>(interface, ip_address, port, family);
		// this->endpoints.emplace_back(endpoint);
		endpoint->start_listening(10);
		this->endpoints.insert(std::make_pair(endpoint->getSocket(), reinterpret_cast<IServerEndpoint*>(endpoint)));
        this->_poll_handler.addSocket(endpoint->getSocket());

		// attach the interface to the endpoint
		// interface->attachToEndpoint(&(*this->endpoints.rbegin()));
		interface->attachToEndpoint(endpoint);

		this->running = true;
	}

	template<class I>
	typename std::enable_if<std::is_base_of<IGatewayInterface<Side::SERVER>, I>::value, void>::type
	addEndpoint(
		I& interface,
	    const std::string& ip_address,
	    const int port,
	    const sa_family_t family = AF_INET
	)
	{
		// create an endpoint for the specified interface
		// ServerEndpoint endpoint = ServerEndpoint(reinterpret_cast<IGatewayInterface<Side::SERVER>*>(&interface), ip_address, port, family);
		ServerEndpoint<typename I::protocol_type> *endpoint = new ServerEndpoint<typename I::protocol_type>(&interface, ip_address, port, family);
		endpoint->start_listening(10); // todo max_pending_connections
		this->endpoints.insert(std::make_pair(endpoint->getSocket(), reinterpret_cast<IServerEndpoint*>(endpoint)));
        this->_poll_handler.addSocket(endpoint->getSocket());
		//
		//this->endpoints//.emplace_back(endpoint);

		// attach the interface to the endpoint
		// interface.attachToEndpoint(&(*this->endpoints.rbegin()));
		interface.attachToEndpoint(endpoint);

		this->running = true;
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
    

	/* ================================================ */
	/* Other Utils                                      */
	/* ================================================ */

	/* disconnects the specified client from the server*/
	bool    disconnect(IServerClient* client);

	/* shutdowns the server an closes all connections */
	void    shutdown();

	// finds a client by searching all the endpoints
	// this is quite slow 
	IServerClient* findClient(int socket);

	// queues data to be sent to client 
	// this call must be used by a protocol to emit their formatted data
	void    sendData(IServerClient* client, const void *data, size_t data_size);


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

	int     _accept(IServerEndpoint* endpoint);
	

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
// TODO MOVE THIS ELSEWHERE MORE VISIBLE
#define RECV_BLK_SIZE   1024
	bool     _receive(IServerClient* from);

	// Sends the data queued for client, returns true if data was flushed entierly.
	bool    _send_data(IServerClient* client);


    /* ================================================ */
    /* Public attributes                                */
    /* ================================================ */

    public:
	//std::list<ServerEndpoint>   endpoints;
	std::map<int, IServerEndpoint*>	endpoints;

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