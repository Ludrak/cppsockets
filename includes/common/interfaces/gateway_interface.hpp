#pragma once

#include "common/packets/packet_parser.hpp"
#include "common/protocols/protocol.hpp"
#include <vector>

/***********************************************************************************/
/*                            GATEWAY INTERFACES                                   */
/***********************************************************************************/

template<Side T>
class GatewayInterfaceBase {};

template<>
class GatewayInterfaceBase<Side::CLIENT>
{
    public:
        GatewayInterfaceBase(Client& client, PacketParserBase* parser, ProtocolBase* protocol)
        : parser(parser), protocol(protocol), client(client), connection()
        {}
        virtual ~GatewayInterfaceBase() {}

        /* Base hooks, called by the server to interface with the protocol */
        virtual void    onConnected() = 0;
        virtual void    onDisconnected() = 0;

        virtual void    receive(void* parsed_data) = 0;

        PacketParserBase*   getParser() { return (parser); };
        ProtocolBase*       getProtocol() { return (protocol); }

        void                attachToConnection(ClientConnection* connection) { this->connection.push_back(connection); }
        void                detachFromConnection(unsigned int idx)
        {
            if (idx < this->connection.size())
                this->connection.erase(this->connection.begin() + idx);
        }
    
    protected:
        PacketParserBase* 	parser;
		ProtocolBase*	    protocol;
    
    public:
        Client&             client;
        std::vector<ClientConnection*>    connection;
};

template<>
class GatewayInterfaceBase<Side::SERVER>
{
    public:
        GatewayInterfaceBase(Server& server, PacketParserBase* parser, ProtocolBase* protocol)
        : parser(parser), protocol(protocol), server(server), endpoint()
        {}

        virtual ~GatewayInterfaceBase() {}

        /* Base hooks, called by the server to interface with the protocol */
        virtual void    onConnected(ServerClient& client) = 0;
        virtual void    onDisconnected(ServerClient& client) = 0;

        virtual void    receive(ServerClient& from, void* parsed_data) = 0;
        
        PacketParserBase*   getParser() { return (parser); };
        ProtocolBase*       getProtocol() { return (protocol); }

        void                attachToEndpoint(ServerEndpoint* endpoint) { this->endpoint.emplace_back(endpoint); }
        void                detachFromEndpoint(unsigned int idx)
        {
            if (idx < this->endpoint.size())
                this->endpoint.erase(this->endpoint.begin() + idx);
        }
    protected:
        PacketParserBase*	parser;
    	ProtocolBase*	    protocol;

    public:
        Server&             server;
        std::vector<ServerEndpoint*>  endpoint;
};


template<Side S, Packets P>
class GatewayInterface
{
    typedef PacketParser<P>			parser_type;
    typedef GatewayInterface<S, P>	interface_type;
};

