#pragma once

#include "sys/socket.h"
#include <map>
#include <arpa/inet.h>
#include <string>
#include <exception>

#include "common/tcp_socket.hpp"
#include "common/inet_address.hpp"

#include "common/protocols/protocol.hpp"

#include "common/protocols/packet_manager.hpp"

class Client;

class ClientConnection : public InetAddress, public TcpSocket, public PacketManager
{
	public:

#ifdef ENABLE_TLS
		ClientConnection(const std::string& ip_address, const int port, const bool useTLS = false, const sa_family_t family = AF_INET);
#else
		ClientConnection(const std::string& ip_address, const int port, const sa_family_t family = AF_INET);
#endif

#ifdef ENABLE_TLS
		bool								useTLS() const;
        SSL*                                getSSL() const { return this->_ssl_connection; }
        bool                                useTLS() const { return this->_useTLS; }
        std::string                         getCertificate();
#endif

		void    							connect() const;
		void								close();

		// switches the current interface of the endpoint
		void								setInterface(GatewayInterfaceBase<Side::CLIENT> *const interface);
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
