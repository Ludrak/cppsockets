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

template<class ...T>
class ServerClient;

class IServerClient;


class IServerEndpoint : public InetAddress, public Socket<Side::SERVER>
{
	friend class Server;

	public:
		IServerEndpoint(
			const IProtocolMethod<Side::SERVER> *const method,
			const std::string& ip_address,
			const int port,
			const sa_family_t family = AF_INET
		) 
		: InetAddress(ip_address, port, family), Socket(method)
		{}

		virtual ~IServerEndpoint() {}

		virtual void	start_listening(const int max_pending_connections) const = 0;
		virtual void	close() = 0;

		virtual IServerClient*	createClient(int socket, InetAddress& addr) = 0;
		virtual bool			delClient(int socket) = 0;

		virtual IServerClient*	getClient(int socket) = 0;

		virtual IGatewayInterface<Side::SERVER>*	getInterface() = 0;

};


template<class Proto>
class ServerEndpoint : public IServerEndpoint, public ClientsManager<Proto>
{
	friend class Server;

	public:
		ServerEndpoint(
			GatewayInterface<Side::SERVER, Proto>* interface,
			const std::string& ip_address,
			const int port,
			const sa_family_t family = AF_INET
		)
		: IServerEndpoint(interface->getProtocol(), ip_address, port, family), _interface(interface)
		{}

		~ServerEndpoint() {}



#ifdef ENABLE_TLS
		bool								useTLS() const;
#endif


		void    							start_listening(const int max_pending_connections) const override
		{
			if (this->_address_family == AF_INET)
			{
				if (bind(this->getSocket(), (sockaddr*)(&this->_address_4), sizeof(this->_address_4)) != 0)
				{
					// LOG_ERROR(LOG_CATEGORY_NETWORK, "Bind syscall failed for address initialization on port " << this->_port << ": " << strerror(errno));
					throw BindException();
				}
			}
			else if (this->_address_family == AF_INET6)
			{
				if (bind(this->getSocket(), (sockaddr*)(&this->_address_6), sizeof(this->_address_6)) != 0)
				{
					// LOG_ERROR(LOG_CATEGORY_NETWORK, "Bind syscall failed for address on IPv6 initialization on port " << this->_port << ": " << strerror(errno));
					throw BindException();
				}
			}

			if (listen(this->getSocket(), max_pending_connections) != 0)
			{
				// LOG_ERROR(LOG_CATEGORY_NETWORK, "Listen syscall failed for address initialization on port " << this->_port << ": " << strerror(errno));
				throw ListenException();
			}
		#ifdef ENABLE_TLS
			if (this->_useTLS)
				;// LOG_INFO(LOG_CATEGORY_NETWORK, "Started listening on TLS endpoint " <<  this->getHostname() << " on port " << this->_port)
			else
		#endif
				// LOG_INFO(LOG_CATEGORY_NETWORK, "Started listening on endpoint " << this->getHostname() << " on port " << this->_port)
			std::cout << "Started listening on endpoint " << this->getHostname() << " on port " << this->_port << std::endl;
		}

		void								close() override
		{
			this->clear();
			this->Socket::close();
		}


		IServerClient*	createClient(int socket, InetAddress& addr) override
		{
			std::cout << "ServerEndpoint::createClient()" << std::endl;
			return (this->ClientsManager<Proto>::addClient(reinterpret_cast<IServerEndpoint*>(this), socket, addr));
		}

		bool			delClient(int socket) override
		{
			this->ClientsManager<Proto>::delClient(socket);
			return true; // TODO: handle possible miss on deletion
		}


		IServerClient*	getClient(int socket) override
		{
			return this->ClientsManager<Proto>::getClient(socket);
		}


		// switches the current interface of the endpoint
		IGatewayInterface<Side::SERVER>*	getInterface() override
		{
			return (this->_interface);
		}

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

		GatewayInterface<Side::SERVER, Proto>*		_interface;
};

#include "server_client.hpp"
