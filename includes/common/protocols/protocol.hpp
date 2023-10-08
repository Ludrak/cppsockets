#pragma once

#include <cstdlib>
#include "protocol_list.hpp"
#include "common/side.hpp"


// #include "server/server_client.hpp"
// #include "server/server_endpoint.hpp"

// Predefinition of ServerClient class
class ServerClient;

// Predefintion of ServerEndpoint class
class ServerEndpoint;

// Predefinition of ClientConnection class
class ClientConnection;

// Predefintion of Client class
class Client;

// Predefinition of Server class
class Server;



/***********************************************************************************/
/*                            PROTOCOL PARSERS                                     */
/***********************************************************************************/

class ProtocolParserBase
{
	public:
        virtual ~ProtocolParserBase() {}

		enum class EvalResult {
			// the strip failed the requirements for this protocol
			INVALID,
			// the strip seems valid, however more data is exepted 
			INCOMPLETE,
			// the packet is complete, and can be parsed
			COMPLETE,
		};

    public:
        /* Evaluates the data strip, returns a status of the currently received packet */
        virtual EvalResult	eval(const char *data_strip, const size_t strip_size) = 0;

        /* parses a completed packet that should have been evaluated as 'complete' by eval*/
        virtual void*		parse(const char *data_strip, const size_t strip_size) = 0;
};

template<Protocol P>
class ProtocolParser {};


/***********************************************************************************/
/*                            GATEWAY INTERFACES                                   */
/***********************************************************************************/

template<Side T>
class GatewayInterfaceBase {};

template<>
class GatewayInterfaceBase<Side::CLIENT>
{
    public:
        GatewayInterfaceBase(Client& client, ClientConnection& connection, ProtocolParserBase* parser)
        : parser(parser), client(client), connection(connection)
        {}
        virtual ~GatewayInterfaceBase() {}

        /* Base hooks, called by the server to interface with the protocol */
        virtual void    onConnected() = 0;
        virtual void    onDisconnected() = 0;

        virtual void    receive(void* parsed_data) = 0;

        ProtocolParserBase* getParser() { return (parser); };
    
    protected:
        ProtocolParserBase* parser;
    
    public:
        Client&             client;
        ClientConnection&   connection;
};

template<>
class GatewayInterfaceBase<Side::SERVER>
{
    public:
        GatewayInterfaceBase(Server& server, ServerEndpoint& endpoint, ProtocolParserBase* parser)
        : parser(parser), server(server), endpoint(endpoint)
        {}

        virtual ~GatewayInterfaceBase() {}

        /* Base hooks, called by the server to interface with the protocol */
        virtual void    onConnected(ServerClient& client) = 0;
        virtual void    onDisconnected(ServerClient& client) = 0;

        virtual void    receive(ServerClient& from, void* parsed_data) = 0;
        
        ProtocolParserBase* getParser() { return (parser); };
    
    protected:
        ProtocolParserBase* parser;
    
    public:
        Server&             server;
        ServerEndpoint&     endpoint;
};


template<Side S, Protocol P>
class GatewayInterface
{
    typedef ProtocolParser<P>       parser_type;
    typedef GatewayInterface<S, P>  interface_type;
};



