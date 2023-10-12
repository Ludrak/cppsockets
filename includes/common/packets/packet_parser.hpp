#pragma once

#include <cstdlib>

#include "common/packets/packet_parser_list.hpp"
#include "common/side.hpp"


// #include "server/server_client.hpp"
// #include "server/server_endpoint.hpp"

template<Side S, class Proto>
class GatewayInterface;

// Predefinition of ServerClient class
template<class ...T>
class ServerClient;
class IServerClient;

// Predefintion of ServerEndpoint class
template<class Proto>
class ServerEndpoint;
class IServerEndpoint;

// Predefinition of ClientConnection class
template<class Proto, class ...T>
class ClientConnection;
class IClientConnection;

// Predefintion of Client class
class Client;

// Predefinition of Server class
class Server;



/***********************************************************************************/
/*                            PROTOCOL PARSERS                                     */
/***********************************************************************************/

#include "common/protocols/protocol.hpp"

template<Side S>
class IPacketParser : public ProtocolMethod<S, Protocols::Transport::TCP>
{
	public:
        virtual ~IPacketParser() {}

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

// template<class Proto>
// class PacketParser {};



