#pragma once

#include <map>

class ServerClient;
class Server;

class ClientsManager
{
    protected:
        ServerClient&						getClient(int socket);
        std::map<int, ServerClient>&		getClients();

        ServerClient*						addClient(const ServerClient& client);
        void								delClient(int socket);

        void                                clear();
    
    private:
		std::map<int, ServerClient>		    _clients;
};