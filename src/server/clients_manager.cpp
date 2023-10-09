#include "server/server_client.hpp"
#include "server/clients_manager.hpp"

ServerClient&				    ClientsManager::getClient(int socket)
{
	return (this->_clients.find(socket)->second);
}

std::map<int, ServerClient>&    ClientsManager::getClients()
{
	return (this->_clients);
}


ServerClient*				    ClientsManager::addClient(const ServerClient& client)
{
	// TODO do a better insertion here 
	std::pair<std::map<int, ServerClient>::iterator, bool> insert = std::make_pair(this->_clients.end(), false);
	try {
		insert = this->_clients.insert(std::make_pair(client.getSocket(), client));
	} catch (std::exception e)
	{
		return (nullptr);
	}
	return (&insert.first->second);
}

void						    ClientsManager::delClient(int socket)
{
	this->_clients.erase(socket);
}


void                            ClientsManager::clear()
{
    this->_clients.clear();
}