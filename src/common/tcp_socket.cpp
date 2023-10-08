#include "common/tcp_socket.hpp"


TcpSocket::TcpSocket(const int socket, const sa_family_t family) 
: _socket(socket), _family(family)
{ }


TcpSocket::TcpSocket(const sa_family_t family) 
{
	this->_socket = ::socket(family, SOCK_STREAM, 0);
	if (this->_socket <= 0)
		throw std::logic_error(std::string("Unable to create socket: ") + strerror(errno));
}


int     TcpSocket::getSocket() const
{
	return (this->_socket);
}


int     TcpSocket::getFamily() const
{
	return (this->_family);
}


void    TcpSocket::close()
{
	::close(this->_socket);
	this->_socket = -1;
}