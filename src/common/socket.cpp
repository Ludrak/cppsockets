// 	#include "common/socket.hpp"
// template<Side S>
// Socket<S>::Socket(const int socket, const sa_family_t family)
// {
// 	this->_socket = socket;
// 	this->_family = family;
// }
// template<Side S>
// Socket<S>::Socket(const IProtocolMethod<S>* method)
// {
// 	this->_socket = method->generateSocket();
// }

// template<Side S>
// int     Socket<S>::getSocket() const
// {
// 	return (this->_socket);
// }

// template<Side S>
// int     Socket<S>::getFamily() const
// {
// 	return (this->_family);
// }

// template<Side S>
// void    Socket<S>::close()
// {
// 	::close(this->_socket);
// 	this->_socket = -1;
// }