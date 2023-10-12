#pragma once

#include <map>

template<class ...T>
class ServerClient;
class IServerClient;

class Server;


// template<class ...T>
// class ClientWrap
// {
//     public:
//         ClientWrap(const ServerClient& client)
//         : client(client), data()
//         {}

//         ServerClient    client;
//         Data<T...>      data;
// };

template<class Proto>
class ClientsManager
{
    protected:
        typedef typename GatewayInterface<Side::SERVER, Proto>::client_type client_type;

        // typename GatewayInterface<Side::SERVER, Proto>::client_type* getClient(int socket)
        // {
        //     std::map<int, GatewayInterface<Side::SERVER, Proto>::client_type>::iterator it = this->_clients.find(socket);
        //     if (it == this->_clients.end())
        //         return (nullptr);
        //     return (&it->second);
        // }

        IServerClient*					                    getClient(int socket)
        {
            //typename std::map<int, typename GatewayInterface<Side::SERVER, Proto>::client_type>::iterator 
            auto it = this->_clients.find(socket);

            if (it == this->_clients.end())
                return (nullptr);
            return (reinterpret_cast<IServerClient*>(&it->second));
        }

        std::map<int, typename GatewayInterface<Side::SERVER, Proto>::client_type>&		getClients() const
        {
            return (this->_clients);
        }

        IServerClient*						addClient(IServerEndpoint *const endpoint, const int socket, const InetAddress& addr)
        {
        	std::cout << "ClientsManager::addClient()" << std::endl;
            std::pair<typename std::map<int, client_type>::iterator, bool>
                insertion = this->_clients.insert(std::make_pair(socket, client_type(endpoint, socket, addr)));
            if (insertion.second == false)
                return (nullptr);
            //std::cout << "returned inserted client endpoint : "<< (insertion.first)..getEndpoint() << std::endl;
            return (reinterpret_cast<IServerClient*>(&(insertion.first->second)));
        }

        void								delClient(int socket)
        {
            this->_clients.erase(socket);
        }

        void                                clear()
        {
            this->_clients.clear();
        }
    
    private:
		std::map<int, client_type>       _clients;
};