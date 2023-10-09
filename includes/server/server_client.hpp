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

// predefinition of ServerEndpoint
class ServerEndpoint;

// predefinition of ServerClient
class Server;



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
class ServerClient : public InetAddress, public Socket, public BufferedIO
{
    public:

#ifdef ENABLE_TLS
        ServerClient(ServerEndpoint& accepted_ep, const int socket, const InetAddress& addr_info, SSL* ssl = nullptr);
#else
        ServerClient(ServerEndpoint& accepted_ep, const int socket, const InetAddress& addr_info);
#endif

        ServerClient(const ServerClient& copy);

        ServerEndpoint& getEndpoint();


#ifdef ENABLE_TLS
        SSL*        getSSL() const { return this->_ssl_connection; }
        bool        useTLS() const { return this->_useTLS; }
        std::string getCertificate();
#endif




#ifdef ENABLE_TLS
    private:
        SSL         *_ssl_connection;
        const bool  _useTLS;
        bool        _accept_done;
#endif
        ServerEndpoint& _endpoint;

    friend class Server;
};

#include "server_endpoint.hpp"