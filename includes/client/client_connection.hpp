#pragma once

#include "sys/socket.h"
#include <map>
#include <arpa/inet.h>
#include <string>
#include <exception>

#include "common/socket.hpp"
#include "common/inet_address.hpp"

#include "common/interfaces/gateway_interface.hpp"
#include "common/packets/buffered_io.hpp"

class Client;

class ClientConnection : public InetAddress, public Socket, public BufferedIO
{
	public:
		ClientConnection(
			GatewayInterfaceBase<Side::CLIENT> *const interface,
			const std::string& ip_address,
			const int port,
			const sa_family_t family = AF_INET
		);



#ifdef ENABLE_TLS
		bool								useTLS() const;
        SSL*                                getSSL() const { return this->_ssl_connection; }
        bool                                useTLS() const { return this->_useTLS; }
        std::string                         getCertificate();
#endif

		void    							connect() const;
		void								close();

		// switches the current interface of the endpoint
		GatewayInterfaceBase<Side::CLIENT>*	getInterface();

		class ConnectException : public std::logic_error
		{
			public:
			ConnectException() : std::logic_error("connection exception occured: abort") {}
		};

	private:
#ifdef ENABLE_TLS
		bool        _useTLS;
        SSL         *_ssl_connection;
        const bool  _useTLS;
        bool        _accept_done;
#endif

		GatewayInterfaceBase<Side::CLIENT>*			_interface;
};
