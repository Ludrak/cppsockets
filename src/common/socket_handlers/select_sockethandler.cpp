
#include "common/socket_handlers/select_sockethandler.hpp"


SocketsHandler::SocketsHandler()
{
	FD_ZERO(&this->_read_fds);
	FD_ZERO(&this->_write_fds);
}



// Adds the socket to the list of checked sockets
int		SocketsHandler::addSocket(const int socket)
{
	// select for reading by default
	FD_SET(socket, &this->_read_fds);
	this->_selected_fds.push_back(socket);
	return (0);
}



// Deletes the socket from the list of checked sockets
int		SocketsHandler::delSocket(const int socket)
{
	FD_CLR(socket, &this->_read_fds);
	FD_CLR(socket, &this->_write_fds);
	for (std::vector<int>::iterator it = this->_selected_fds.begin(); it != this->_selected_fds.end(); ++it)
	{
		if (*it == socket)
		{
			this->_selected_fds.erase(it);
			return 0;
		}
	}
	throw (std::logic_error("Deleted socket was not in pollfd list."));
}



// Set the socket flag for reading for socket depending on selected
int		SocketsHandler::socketWantsRead(const int socket, bool selected)
{
	if (selected)
		FD_SET(socket, &this->_read_fds);
	else
		FD_CLR(socket, &this->_read_fds);
	return (0);
}



// Set the socket flag for writing for socket depending on selected
int		SocketsHandler::socketWantsWrite(const int socket, bool selected)
{
	if (selected)
		FD_SET(socket, &this->_write_fds);
	else
		FD_CLR(socket, &this->_write_fds);
	return (0);
}



// process a poll on the given sockets list
bool   SocketsHandler::processPoll(const int timeout_ms)
{
	// preserve main fd sets
	FD_ZERO(&this->_selected_read_fds);
	FD_ZERO(&this->_selected_write_fds);
	FD_COPY(&this->_read_fds, &this->_selected_read_fds);
	FD_COPY(&this->_write_fds, &this->_selected_write_fds);
	
	int nfds = this->_getnfds();
	struct timeval *timeout_ptr = nullptr;
	struct timeval timeout = {.tv_sec=timeout_ms / 1000,.tv_usec=(timeout_ms % 1000) * 1000};
	if (timeout_ms >= 0)
	{
		timeout_ptr = &timeout;
	}
	if (select(nfds, &this->_selected_read_fds, &this->_selected_write_fds, nullptr, timeout_ptr) < 0)
	{
		return (true);
	}

	// change poll id on success
	// this value can overflow, but for safety clamp it to 32
	this->_poll_id = ++this->_poll_id % 32;
	return false;
}



SocketsHandler::socket_event    SocketsHandler::nextSocketEvent()
{
	static size_t n = 0;
	static size_t poll_id = 0;

	// reset n for each new poll
	if (this->_poll_id != poll_id)
	{
		poll_id = this->_poll_id;
		n = 0;
	}

	if (n == this->_selected_fds.size())
	{
		return (_SOCKET_END);
	}
	while (n < this->_selected_fds.size())
	{
		int sock = this->_selected_fds.at(n);
		int events = 0;
		
		if (FD_ISSET(sock, &this->_selected_read_fds))
			events |= _SOCK_READ;
		if (FD_ISSET(sock, &this->_selected_write_fds))
			events |= _SOCK_WRITE;
		
		if (events != 0)
		{
			++n;
			return SocketsHandler::socket_event(sock, events);
		}
		++n;
	}
	return (_SOCKET_END);
}



int		SocketsHandler::_getnfds()
{
	static size_t nfds = 0;
	static size_t last_sock_count = -1;

	if (last_sock_count != this->_selected_fds.size())
	{
		last_sock_count = this->_selected_fds.size();
		for (int i : this->_selected_fds)
		{
			nfds = std::max(nfds, (size_t)i);
		}
	}

	return (nfds + 1);
}