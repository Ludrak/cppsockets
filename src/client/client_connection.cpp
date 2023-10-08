#include "client/client_connection.hpp"

#ifdef ENABLE_TLS

ClientConnection::ClientConnection(const std::string& ip_address, const int port, const bool useTLS, const sa_family_t family)
: InetAddress(ip_address, port, family), TcpSocket(family), _interface(nullptr), _parser(nullptr), _useTLS(useTLS)
{}

#else

ClientConnection::ClientConnection(const std::string& ip_address, const int port, const sa_family_t family)
: InetAddress(ip_address, port, family), TcpSocket(family), _interface(nullptr)
{}

#endif




#ifdef ENABLE_TLS
bool    ClientConnection::useTLS() const
{
	return (this->_useTLS);
}
#endif


void    ClientConnection::connect() const
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

void			                    ClientConnection::close()
{
	this->TcpSocket::close();
}


void								ClientConnection::setInterface(GatewayInterfaceBase<Side::CLIENT> *const interface)
{
	this->_interface = interface;
}

GatewayInterfaceBase<Side::CLIENT>*	ClientConnection::getInterface()
{
	return (this->_interface);
}


