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


#include "client/client_connection.hpp"

class Client
{
    public:
	/* ================================================ */
	/* Constructors                                     */
	/* ================================================ */

	// default constructor
	Client(void);

	/* Client constructor for multiple connections                               */
	/* this call can throw if one of the connectiond contains an invalid address */
	/* and thus needs to be catched.                                             */
	/* If TLS is enabled, it can also throw a SSLInitException.                  */
	Client(const std::list<ClientConnection>& connections);

	/* Client constructor for single connection                                  */
	/* this call can throw if the connection contains an invalid address.        */
	/* If TLS is enabled, it can also throw a SSLInitException.                */


/* Client constructor for single connection                                  */
/* this call can throw if the connection contains an invalid address.        */
/* If TLS is enabled, it can also throw a SSLInitException.                */
	template<class I>
#ifdef ENABLE_TLS
	Client(
		typename std::enable_if<std::is_base_of<GatewayInterfaceBase<Side::CLIENT>, I>::value, const std::string&>::type
		ip_address,
		const int port,
		const bool useTLS,
		const sa_family_t family)
	: Client()
	{
		this->addEndpoint<I>(ip_address, port, useTLS, family);
	}
#else
	Client(
		typename std::enable_if<std::is_base_of<GatewayInterfaceBase<Side::CLIENT>, I>::value, const std::string&>::type
		ip_address,
		const int port,
		const sa_family_t family)
	: Client()
	{
	    this->connect<I>(ip_address, port, family);
	}
#endif
	
	Client(const Client& copy) = delete;

	Client& operator=(const Client& other) = delete;

	~Client();


	/* ================================================ */
	/* Public Members                                   */
	/* ================================================ */

	/* Adds a new ClientConnection with the provided GatewayInterface             */
	template<class I>
	typename std::enable_if<std::is_base_of<GatewayInterfaceBase<Side::CLIENT>, I>::value, void>::type
	connect(
	    const std::string& ip_address,
	    const int port,
	    const sa_family_t family = AF_INET
	)
	{
	    // ClientConnection connection = ClientConnection(ip_address, port, family);
		// this->connections.emplace_back(connection);
		// ClientConnection& insert = *this->connections.rbegin();
	    // GatewayInterfaceBase<Side::CLIENT>* interface = new I(*this, insert);
	    // insert.setInterface(interface);
	    // insert.connect();
		// this->_poll_handler.addSocket(insert.getSocket());

		// create interface
		GatewayInterfaceBase<Side::CLIENT>* interface = new I(*this);
		// create connection of the interface type
		ClientConnection connection = ClientConnection(interface, ip_address, port, family);
		this->connections.emplace_back(connection);
		ClientConnection& insert = *this->connections.rbegin();
		
		// attach connection to the interface
		interface->attachToConnection(&insert);

		// try connect to the server
		insert.connect();

		// add to poll handler
		this->_poll_handler.addSocket(insert.getSocket());
	}

	template<class I>
	typename std::enable_if<std::is_base_of<GatewayInterfaceBase<Side::CLIENT>, I>::value, void>::type
	connect(
		I& interface,
	    const std::string& ip_address,
	    const int port,
	    const sa_family_t family = AF_INET
	)
	{
		// create connection of the interface type
		ClientConnection connection = ClientConnection(reinterpret_cast<GatewayInterfaceBase<Side::CLIENT>*>(&interface), ip_address, port, family);
		this->connections.emplace_back(connection);
		ClientConnection& insert = *this->connections.rbegin();
		
		// attach connection to the interface
		interface->attachToConnection(&insert);

		// try connect to the server
		insert.connect();

		// add to poll handler
		this->_poll_handler.addSocket(insert.getSocket());
	}


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

	/* shutdowns the client an closes all connections */
	void    shutdown();

	// finds a connection by socket
	ClientConnection* findConnection(int socket);

	void	closeConnection(ClientConnection& connection);

	// queues data to be sent to client 
	// this call must be used by a protocol to emit their formatted data
	void	sendData(ClientConnection& connection, const void *data, size_t data_size);



    /* ================================================ */
    /* Private members                                  */
    /* ================================================ */
    private:

	// for the clients that are already connected to an connection
	// returns:
	// true: no errors occured
	// false: an error occured, running needs to be checked in wait_update in case 
	//        a shutdown() was called
	bool    _handleClientEvent(const SocketsHandler::socket_event& ev);

	/* ================================================ */
	/* Connection to server                             */
	/* ================================================ */
// 	bool    _connect(ClientConnection& connection);

// #ifdef ENABLE_TLS
// 	bool    _connect_ssl(ClientConnection& connection);
// #endif



	/* ================================================ */
	/* Recv handler & unpacker                          */
	/* ================================================ */

// must at least be sizeof(packet_data_header) (or sizeof(size_t) + 32)
// TODO MOVE THIS ELSEWHERE MORE VISIBLE
#define RECV_BLK_SIZE   1024
	bool     _receive(ClientConnection& from);

	// Sends the data queued for client, returns true if data was flushed entierly.
	bool    _send_data(ClientConnection& connection);


    /* ================================================ */
    /* Public attributes                                */
    /* ================================================ */

    public:
	std::list<ClientConnection>   connections;

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