#include "common/socket.hpp"


Socket::Socket(const int socket, const sa_family_t family) 
: _socket(socket), _family(family)
{ }


Socket::Socket(const sa_family_t family) 
{
	this->_socket = ::socket(family, SOCK_STREAM, 0);
	if (this->_socket <= 0)
		throw std::logic_error(std::string("Unable to create socket: ") + strerror(errno));
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