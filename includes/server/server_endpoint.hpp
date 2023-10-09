#pragma once

#include "sys/socket.h"
#include <map>
#include <arpa/inet.h>
#include <string>
#include <exception>

#include "common/socket.hpp"
#include "common/inet_address.hpp"

#include "common/interfaces/gateway_interface.hpp"
#include "common/protocols/protocol.hpp"

#include "server/clients_manager.hpp"

class ServerClient;

class ServerEndpoint : protected ClientsManager, public InetAddress, public Socket
{
	public:
		ServerEndpoint(
			GatewayInterfaceBase<Side::SERVER> *const interface,
			const std::string& ip_address,
			const int port,
			const sa_family_t family = AF_INET
		);


#ifdef ENABLE_TLS
		bool								useTLS() const;
#endif


		void    							start_listening(const int max_pending_connections) const;
		void								close();

		// switches the current interface of the endpoint
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

		friend class Server;
};

#include "server_client.hpp"
