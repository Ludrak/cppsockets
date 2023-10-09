#include "server/server_endpoint.hpp"

// #ifdef ENABLE_TLS

// ServerEndpoint::ServerEndpoint(const std::string& ip_address, const int port, const bool useTLS, const sa_family_t family)
// : InetAddress(ip_address, port, family), Socket(family), _interface(nullptr), _parser(nullptr), _useTLS(useTLS), _clients()
// {}

// #else

// ServerEndpoint::ServerEndpoint(const std::string& ip_address, const int port, const sa_family_t family)
// : InetAddress(ip_address, port, family), Socket( /** !!! **/  _interface->getProtocol() /** !!!! **/  ), _interface(nullptr), _clients()
// {}

// #endif

ServerEndpoint::ServerEndpoint(GatewayInterfaceBase<Side::SERVER> *const interface, const std::string& ip_address, const int port, const sa_family_t family)
: ClientsManager(), InetAddress(ip_address, port, family), Socket(interface->getProtocol()), _interface(interface)
{}


#ifdef ENABLE_TLS
bool    ServerEndpoint::useTLS() const
{
	return (this->_useTLS);
}
#endif


void    ServerEndpoint::start_listening(const int max_pending_connections) const
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

void			ServerEndpoint::close()
{
	this->clear();
	this->Socket::close();
}









// ServerClient&						ServerEndpoint::getClient(int socket)
// {
// 	return (this->_clients.find(socket)->second);
// }

// std::map<int, ServerClient>&		ServerEndpoint::getClients()
// {
// 	return (this->_clients);
// }


// ServerClient*						ServerEndpoint::addClient(int socket, const ServerClient& client)
// {
// 	// TODO do a better insertion here 
// 	std::pair<std::map<int, ServerClient>::iterator, bool> insert = std::make_pair(this->_clients.end(), false);
// 	try {
// 		insert = this->_clients.insert(std::make_pair(socket, client));
// 	} catch (std::exception e)
// 	{
// 		return (nullptr);
// 	}
// 	return (&insert.first->second);
// }

// void								ServerEndpoint::delClient(int socket)
// {
// 	this->_clients.erase(socket);
// }


GatewayInterfaceBase<Side::SERVER>*	ServerEndpoint::getInterface()
{
	return (this->_interface);
}

// PacketParserBase*					ServerEndpoint::getParser()
// {
// 	return (this->_parser);
// }

