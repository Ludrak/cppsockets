#pragma once

#include "sys/socket.h"
#include <map>
#include <arpa/inet.h>
#include <string>
#include <exception>

#include "common/socket.hpp"
#include "common/inet_address.hpp"

#include "common/interfaces/gateway_interface.hpp"

class ServerClient;

class ServerEndpoint : public InetAddress, public Socket
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
		std::map<int, ServerClient>					_clients;
};

#include "server_client.hpp"
