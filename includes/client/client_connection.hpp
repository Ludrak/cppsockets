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


class IClientConnection :  public InetAddress, public Socket<Side::CLIENT>, public BufferedIO
{
	public:
		IClientConnection(
			IProtocolMethod<Side::CLIENT> *const method,
			const std::string& ip_address,
			const int port,
			const sa_family_t family = AF_INET
		) : InetAddress(ip_address, port, family), Socket(method) //, BufferedIO()
		{}

		virtual void	connect() const = 0;
		virtual void	close() = 0;


		virtual IGatewayInterface<Side::CLIENT>*	getInterface() = 0;
};

template<class Proto, class ...T>
class ClientConnection : public IClientConnection
{
	public:
		ClientConnection(
			GatewayInterface<Side::CLIENT, Proto>* interface,
			const std::string& ip_address,
			const int port,
			const sa_family_t family = AF_INET
		) : IClientConnection(interface->getProtocol(), ip_address, port, family), _interface(interface), data()
		{}



#ifdef ENABLE_TLS
		bool								useTLS() const;
        SSL*                                getSSL() const { return this->_ssl_connection; }
        bool                                useTLS() const { return this->_useTLS; }
        std::string                         getCertificate();
#endif



		void    							connect() const override
		{
			if (this->_address_family == AF_INET)
			{
				if (::connect(this->getSocket(), (sockaddr*)(&this->_address_4), sizeof(this->_address_4)) != 0)
				{
					// LOG_ERROR(LOG_CATEGORY_NETWORK, "Bind syscall failed for address initialization on port " << this->_port << ": " << strerror(errno));
					throw ConnectException();
				}
			}
			else if (this->_address_family == AF_INET6)
			{
				if (::connect(this->getSocket(), (sockaddr*)(&this->_address_6), sizeof(this->_address_6)) != 0)
				{
					// LOG_ERROR(LOG_CATEGORY_NETWORK, "Bind syscall failed for address on IPv6 initialization on port " << this->_port << ": " << strerror(errno));
					throw ConnectException();
				}
			}
		#ifdef ENABLE_TLS
			if (this->_useTLS)
				;// LOG_INFO(LOG_CATEGORY_NETWORK, "Started listening on TLS endpoint " <<  this->getHostname() << " on port " << this->_port)
			else
		#endif
			// LOG_INFO(LOG_CATEGORY_NETWORK, "Started listening on endpoint " << this->getHostname() << " on port " << this->_port)
			std::cout << "Connected to " << this->getHostname() << " on port " << this->_port << std::endl;
		}

		void								close() override
		{
			this->Socket::close();
		}


		// switches the current interface of the endpoint
		IGatewayInterface<Side::CLIENT>*	getInterface() override
		{
			return reinterpret_cast<IGatewayInterface<Side::CLIENT>*>(this->_interface);
		}

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

		GatewayInterface<Side::CLIENT, Proto>*	_interface;

	public:
		Data<T...>	data;
};
