#pragma once

#include "sys/socket.h"
#include <map>
#include <arpa/inet.h>
#include <string>
#include <exception>

#include "common/tcp_socket.hpp"
#include "common/inet_address.hpp"

#include "common/protocols/protocol.hpp"

class ServerClient;

class ServerEndpoint : public InetAddress, public TcpSocket
{
	public:

#ifdef ENABLE_TLS
		ServerEndpoint(const std::string& ip_address, const int port, const bool useTLS = false, const sa_family_t family = AF_INET);
#else
		ServerEndpoint(const std::string& ip_address, const int port, const sa_family_t family = AF_INET);
#endif

		// those should only be called by the server
		// maybe move to protected and friend class w/ server
		ServerClient&						getClient(int socket);
		std::map<int, ServerClient>&		getClients();

		ServerClient*						addClient(int socket, const ServerClient& client);
		void								delClient(int socket);


#ifdef ENABLE_TLS
		bool								useTLS() const;
#endif


		void    							start_listening(const int max_pending_connections) const;
		void								close();

		// switches the current interface of the endpoint
		void								setInterface(GatewayInterfaceBase<Side::SERVER> *const interface);
		GatewayInterfaceBase<Side::SERVER>*	getInterface();
		//ProtocolParserBase*					getParser();

		class BindException : public std::logic_error
		{
			public:
			BindException() : std::logic_error("bind exception occured: abort") {}
		};

		class ListenException : public std::logic_error
		{
			public:
			ListenException() : std::logic_error("listen exception occured: abort") {}
		};

	private:
#ifdef ENABLE_TLS
		bool    _useTLS;
#endif
		GatewayInterfaceBase<Side::SERVER>*			_interface;
		//ProtocolParserBase*							_parser;

		std::map<int, ServerClient>					_clients;
};

#include "server_client.hpp"

//  USE Server::addEndpoint instead

// #ifdef ENABLE_TLS

// template<Protocol P, GatewayInterface<Side::SERVER, P> I>
// ServerEndpoint*	make_endpoint(
// 	const std::string& ip_address,
// 	const int port,
// 	const bool useTLS = false,
// 	const sa_family_t family = AF_INET,
// 	GatewayInterfaceBase<Side::SERVER>* interface = new I()
// )
// {
// 	return (new ServerEndpoint(interface, ip_address, port, useTLS, family));
// }

// #else

// template<Protocol P, GatewayInterface<Side::SERVER, P> I>
// ServerEndpoint*	make_endpoint(
// 	const std::string& ip_address,
// 	const int port,
// 	const sa_family_t family = AF_INET,
// 	GatewayInterfaceBase<Side::SERVER>* interface = new I()
// )
// {
// 	return (new ServerEndpoint(interface, ip_address, port, family));
// }
// #endif