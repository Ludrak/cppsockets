#pragma once

#include "common/packets/packet_parser.hpp"
#include "common/protocols/protocol.hpp"

/***********************************************************************************/
/*                            GATEWAY INTERFACES                                   */
/***********************************************************************************/

template<Side T>
class GatewayInterfaceBase {};

template<>
class GatewayInterfaceBase<Side::CLIENT>
{
    public:
        GatewayInterfaceBase(Client& client, ClientConnection& connection, PacketParserBase* parser, ProtocolBase* protocol)
        : parser(parser), protocol(protocol), client(client), connection(connection)
        {}
        virtual ~GatewayInterfaceBase() {}

        /* Base hooks, called by the server to interface with the protocol */
        virtual void    onConnected() = 0;
        virtual void    onDisconnected() = 0;

        virtual void    receive(void* parsed_data) = 0;

        PacketParserBase*   getParser() { return (parser); };
        ProtocolBase*       getProtocol() { return (protocol); }
    
    protected:
        PacketParserBase* 	parser;
		ProtocolBase*	    protocol;
    
    public:
        Client&             client;
        ClientConnection&   connection;
};

template<>
class GatewayInterfaceBase<Side::SERVER>
{
    public:
        GatewayInterfaceBase(Server& server, ServerEndpoint& endpoint, PacketParserBase* parser, ProtocolBase* protocol)
        : parser(parser), protocol(protocol), server(server), endpoint(endpoint)
        {}

        virtual ~GatewayInterfaceBase() {}

        /* Base hooks, called by the server to interface with the protocol */
        virtual void    onConnected(ServerClient& client) = 0;
        virtual void    onDisconnected(ServerClient& client) = 0;

        virtual void    receive(ServerClient& from, void* parsed_data) = 0;
        
        PacketParserBase*   getParser() { return (parser); };
        ProtocolBase*       getProtocol() { return (protocol); }

    protected:
        PacketParserBase*	parser;
    	ProtocolBase*	    protocol;

    public:
        Server&             server;
        ServerEndpoint&     endpoint;
};


template<Side S, Packets P>
class GatewayInterface
{
    typedef PacketParser<P>			parser_type;
    typedef GatewayInterface<S, P>	interface_type;
};

