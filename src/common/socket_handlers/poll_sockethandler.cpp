#include "common/socket_handlers/poll_sockethandler.hpp"



// Adds the socket to the list of checked sockets
int		SocketsHandler::addSocket(int socket)
{
	struct pollfd poll_fd;
	poll_fd.fd = socket;
	poll_fd.events = POLLIN; // by default always poll in on new sockets
	poll_fd.revents = 0;
	if (this->_poll_fds.insert(std::make_pair(socket, poll_fd)).second == false)
		throw std::logic_error("Cannot insert pollfd struct in pollfd list.");
	this->_poll_fds_aligned.push_back(poll_fd);
	return (0);
}



// Deletes the socket from the list of checked sockets  
int		SocketsHandler::delSocket(const int socket)
{
	if (this->_poll_fds.erase(socket) == 0)
		throw (std::logic_error("trying to delete a socket that is not in pollfd list."));
	//this->_poll_fds_aligned.erase(this->_poll_fds.find());
	return (0);
}



// Set the socket flag for reading for socket depending on selected
int		SocketsHandler::socketWantsRead(const int socket, bool selected)
{
	fd_map_type::iterator it = this->_poll_fds.find(socket);
	if (it == this->_poll_fds.end())
		throw std::logic_error("socket not found in poll_fds asked for reading: " + std::to_string(socket));
	if (selected)
		it->second.events |= POLLIN;
	else
		it->second.events = it->second.events & ~POLLIN;
	return (0);
}



// Set the socket flag for writing for socket depending on selected
int     SocketsHandler::socketWantsWrite(const int socket, bool selected)
{
	fd_map_type::iterator it = this->_poll_fds.find(socket);
	if (it == this->_poll_fds.end())
		throw std::logic_error("socket not found in poll_fds asked for writing: " + std::to_string(socket));
	if (selected)
		it->second.events |= POLLOUT;
	else
		it->second.events = it->second.events & ~POLLOUT;
	return (0);
}



// process a poll on the given sockets list
bool    SocketsHandler::processPoll(const int timeout_ms)
{

	// WARN very slow
	this->_poll_fds_aligned.clear();

	// only reserve if more capacity is needed
	if (this->_poll_fds_aligned.capacity() < this->_poll_fds.size())
		this->_poll_fds_aligned.reserve(this->_poll_fds.size());

	for (fd_map_type::iterator it = this->_poll_fds.begin(); it != this->_poll_fds.end(); ++it)
		this->_poll_fds_aligned.push_back(it->second);

	if (poll(this->_poll_fds_aligned.data(), this->_poll_fds_aligned.size(), timeout_ms) < 0)
		return true;

	// change poll id on success
	// this value can overflow, but for safety clamp it each 32 polls
	this->_poll_id = ++this->_poll_id % 32;
	return false;
}



SocketsHandler::socket_event	SocketsHandler::nextSocketEvent()
{
	static size_t n = 0;
	static size_t poll_id = 0;

	// reset n for each new poll
	if (this->_poll_id != poll_id)
	{
		poll_id = this->_poll_id;
		n = 0;
	}

	if (n == this->_poll_fds_aligned.size())
	{
		return (_SOCKET_END);
	}
	while (n < this->_poll_fds_aligned.size())
	{
		const pollfd& fd = this->_poll_fds_aligned.at(n);
		if (fd.revents != 0)
		{
			++n;
			return socket_event(fd.fd, fd.revents);
		}
		++n;
	}
	return (_SOCKET_END);
}