#pragma once

#include <arpa/inet.h>
#include <string>

#ifdef ENABLE_TLS
#include <openssl/ssl.h>
#include <openssl/err.h>
#endif

#include "common/packets/buffered_io.hpp"
#include "common/interfaces/gateway_interface.hpp"
#include "common/socket.hpp"
#include "common/inet_address.hpp"

#include "common/data/data.hpp"

// predefinition of ServerEndpoint
template<class Proto>
class ServerEndpoint;

class IServerEndpoint;

// predefinition of ServerClient
class Server;

/* AKA server client base class */
class IServerClient : public InetAddress, public Socket<Side::SERVER>, public BufferedIO
{
    public:
        IServerClient(IServerEndpoint *const accepted_ep, const int socket, const InetAddress& addr_info)
        : InetAddress(addr_info), Socket(socket, addr_info.getAddressFamily()), _endpoint(accepted_ep)
    	{}

    protected:
        // cannot copy client without its data component (ServerClient<...T>)
        // IServerClient(const IServerClient& copy) = delete;

        IServerClient(const IServerClient& copy)
        : InetAddress(copy), Socket(copy), _endpoint(copy.getEndpoint())
        {
            std::cout << "COPY ENDPOINT " << copy.getEndpoint() << std::endl;
        }

    public:

        IServerEndpoint*    getEndpoint() const
        {
            return (this->_endpoint);
        }

//         #ifdef ENABLE_TLS
//         SSL*        getSSL() const { return this->_ssl_connection; }
//         bool        useTLS() const { return this->_useTLS; }
//         std::string getCertificate();
// #endif




// #ifdef ENABLE_TLS
//     private:
//         SSL         	*_ssl_connection;
//         const bool  	_useTLS;
//         bool        	_accept_done;
// #endif
    
    private:
        IServerEndpoint*    _endpoint;
};


/*  ServerClient class 
*
*   TODO DESCRIPTION
*   ...
*
*
*
*
*
*/

/* AKA server client base class + data classes */
template<class ...T>
class ServerClient : public IServerClient
{
    public:

        ServerClient(IServerEndpoint *const accepted_ep, const int socket, const InetAddress& addr_info)
		: IServerClient(accepted_ep, socket, addr_info), data()
    	{
            std::cout << "client constructor" << std::endl;
        } 

        // includes copy to copy all data
        // TODO: fix InetAddress copy
        ServerClient(const ServerClient<T...>& copy)
        : IServerClient(copy), data()
        {
            std::cout << "client copy constructor" << std::endl;
            // good way of copying the data ?
            // std::memcpy(&this->data, &copy.data, sizeof(this->data));
        }

	public:
        Data<T...>		data;

    friend class Server;
};



// class ServerClient : public InetAddress, public Socket, public BufferedIO
// {
//     public:

// // #ifdef ENABLE_TLS
// //         ServerClient(ServerEndpoint& accepted_ep, const int socket, const InetAddress& addr_info, SSL* ssl = nullptr);
// // #else
// //         ServerClient(ServerEndpoint& accepted_ep, const int socket, const InetAddress& addr_info);
// // #endif

//         ServerClient(IServerEndpoint* accepted_ep, const int socket, const InetAddress& addr_info)
// 		: InetAddress(addr_info), Socket(socket, addr_info.getAddressFamily()), _endpoint(accepted_ep)
//     	{} 


//         ServerClient(const ServerClient& copy);

//         IServerEndpoint* getEndpoint();


// #ifdef ENABLE_TLS
//         SSL*        getSSL() const { return this->_ssl_connection; }
//         bool        useTLS() const { return this->_useTLS; }
//         std::string getCertificate();
// #endif




// #ifdef ENABLE_TLS
//     private:
//         SSL         	*_ssl_connection;
//         const bool  	_useTLS;
//         bool        	_accept_done;
// #endif
//         // IServerEndpoint*    _endpoint;

// 	public:
//         Data<T...>		data;

//     friend class Server;
// };

#include "server_endpoint.hpp"