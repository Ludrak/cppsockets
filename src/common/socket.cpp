#include "common/socket.hpp"

Socket::Socket(const int socket, const sa_family_t family)
{
	this->_socket = socket;
	this->_family = family;
}

Socket::Socket(const ProtocolBase* method)
{
	this->_socket = method->generateSocket();
}

int     Socket::getSocket() const
{
	return (this->_socket);
}


int     Socket::getFamily() const
{
	return (this->_family);
}


void    Socket::close()
{
	::close(this->_socket);
	this->_socket = -1;
}