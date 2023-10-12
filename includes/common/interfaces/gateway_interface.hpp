#pragma once

// #include "common/packets/packet_parser.hpp"
#include "common/protocols/protocol.hpp"
#include <vector>

/***********************************************************************************/
/*                            GATEWAY INTERFACES                                   */
/***********************************************************************************/

template<Side S>
class IGatewayInterface
{
};

template<>
class IGatewayInterface<Side::SERVER>
{
    public:
        virtual ~IGatewayInterface() {}

        virtual IProtocolMethod<Side::SERVER>*  getProtocol() = 0;

        virtual void                    attachToEndpoint(IServerEndpoint* endpoint) = 0;
        virtual void                    detachFromEndpoint(int socket) = 0;

        // basic hooks from server
        virtual void                    onConnected(IServerClient* client)  {(void)client;};
        virtual void                    onDisconnected(IServerClient* client)   {(void)client;};
        // virtual void                    onReceived(IServerClient* client, void* parsed_data, const size_t data_size)    {(void)client;(void)parsed_data;(void)data_size;};
};

template<>
class IGatewayInterface<Side::CLIENT>
{
    public:
        virtual ~IGatewayInterface() {}

        virtual IProtocolMethod<Side::CLIENT>*  getProtocol() = 0;

        virtual void                    attachToConnection(IClientConnection* endpoint) = 0;
        virtual void                    detachFromConnection(int socket) = 0;

        // basic hooks from server
        virtual void                    onConnected(IClientConnection* connection)   { (void) connection; };
        virtual void                    onDisconnected(IClientConnection* connection)    { (void) connection; };
        // virtual void                    onReceived(void* parsed_data, const size_t data_size)   {(void)parsed_data;(void)data_size;};
};

// template<>
// class IGatewayInterface<Side::CLIENT>
// {
//     public:
//         IGatewayInterface(Client& client, IPacketParser* parser, IProtocolMethod* protocol)
//         : parser(parser), protocol(protocol), client(client), connection()
//         {}
//         virtual ~IGatewayInterface() {}

//         /* Base hooks, called by the server to interface with the protocol */
//         virtual void    onConnected() = 0;
//         virtual void    onDisconnected() = 0;

//         virtual void    receive(void* parsed_data) = 0;

//         IPacketParser*   getParser() { return (parser); };
//         IProtocolMethod*       getProtocol() { return (protocol); }

//         void                attachToConnection(ClientConnection* connection) { this->connection.push_back(connection); }
//         void                detachFromConnection(unsigned int idx)
//         {
//             if (idx < this->connection.size())
//                 this->connection.erase(this->connection.begin() + idx);
//         }
    
//     protected:
//         IPacketParser* 	parser;
// 		IProtocolMethod*	    protocol;
    
//     public:
//         Client&             client;
//         std::vector<ClientConnection*>    connection;
// };

// template<>
// class IGatewayInterface<Side::SERVER>
// {
//     typedef Data<>                          client_data_type;
//     typedef ServerClient<client_data_type>  client_type;

//     public:
//         IGatewayInterface(Server& server, IPacketParser* parser, IProtocolMethod* protocol)
//         : parser(parser), protocol(protocol), server(server), endpoint()
//         {}

//         virtual ~IGatewayInterface() {}

//         /* Base hooks, called by the server to interface with the protocol */
//         virtual void    onConnected(client_type& client) = 0;
//         virtual void    onDisconnected(client_type& client) = 0;

//         virtual void    receive(client_type& from, void* parsed_data) = 0;
        
//         IPacketParser*      getParser() { return (parser); };
//         IProtocolMethod*    getProtocol() { return (protocol); }

//         void                attachToEndpoint(IServerEndpoint* endpoint) { this->endpoint.emplace_back(endpoint); }
//         void                detachFromEndpoint(unsigned int idx)
//         {
//             if (idx < this->endpoint.size())
//                 this->endpoint.erase(this->endpoint.begin() + idx);
//         }
//     protected:
//         IPacketParser*	parser;
//     	IProtocolMethod*	    protocol;

//     public:
//         Server&             server;
//         std::vector<IServerEndpoint*>  endpoint;
// };

template<Side S, class Proto>
class GatewayInterface
{
};

// template<class Proto>
// class GatewayInterface<Side::SERVER, Proto> : public IGatewayInterface<Side::SERVER>
// {
//     typedef PacketParser<Proto>			            parser_type;
//     typedef GatewayInterface<Side::SERVER, Proto>   interface_type;

//     typedef ProtocolMethod<Side::SERVER, Proto>     protocol_type;
//     typedef protocol_type::data_type                protocol_data_type;
//     typedef ServerClient<protocol_data_type>        client_type;
// };

// template<class Proto>
// class GatewayInterface<Side::CLIENT, Proto> : public IGatewayInterface<Side::CLIENT>
// {
//     typedef PacketParser<Proto>			            parser_type;
//     typedef GatewayInterface<Side::CLIENT, Proto>   interface_type;

//     typedef ProtocolMethod<Side::CLIENT, Proto>     protocol_type;
//     typedef protocol_type::data_type                protocol_data_type;
//     typedef ClientConnection<protocol_data_type>    client_type;
// };


// template<>
// class GatewayInterface<Side::SERVER, Protocols::Transport::TCP> : public IGatewayInterface<Side::SERVER>
// {
//     public:
//        //typedef PacketParser<Protocols::Transport::TCP>			            parser_type;
//         typedef GatewayInterface<Side::SERVER, Protocols::Transport::TCP>   interface_type;

//         typedef Protocols::Transport::TCP                                   protocol_type;
//         typedef ProtocolMethod<Side::SERVER, Protocols::Transport::TCP>     protocol_method_type;
//         typedef protocol_method_type::data_type                             protocol_data_type;
//         typedef ServerClient<protocol_data_type>                            client_type;

//         typedef ServerEndpoint<Protocols::Transport::TCP>                   endpoint_type;

//         IProtocolMethod<Side::SERVER>*     getProtocol() override
//         {
//             return (reinterpret_cast<IProtocolMethod<Side::SERVER>*>(&this->_protocol));
//         }

//         void    attachToEndpoint(IServerEndpoint* endpoint) override
//         {
//             endpoint_type* ep = dynamic_cast<endpoint_type*>(endpoint);
//             if (ep == nullptr)
//                 return ;
//             this->endpoints.push_back(ep);
//         }

//         void    detachFromEndpoint(const int socket) override
//         {
//             for (std::vector<endpoint_type*>::iterator it = this->endpoints.begin(); it != this->endpoints.end(); ++it)
//             {
//                 if ((*it)->getSocket() == socket)
//                 {
//                     this->endpoints.erase(it);
//                     return ;
//                 }
//             }
//         }

//     public: 
//         std::vector<endpoint_type*> endpoints;

//     private:
//         protocol_type               _protocol;
// };

// template<>
// class GatewayInterface<Side::CLIENT, Protocols::Transport::TCP> : public IGatewayInterface<Side::CLIENT>
// {
//     public:
//         //typedef PacketParser<Protocols::Transport::TCP>			            parser_type;
//         typedef GatewayInterface<Side::CLIENT, Protocols::Transport::TCP>   interface_type;

//         typedef Protocols::Transport::TCP                                   protocol_type;
//         typedef ProtocolMethod<Side::CLIENT, Protocols::Transport::TCP>     protocol_method_type;
//         typedef protocol_method_type::data_type                             protocol_data_type;

//         typedef ClientConnection<protocol_type, protocol_data_type>         connection_type;

//         IProtocolMethod<Side::CLIENT>*     getProtocol() override
//         {
//             return (reinterpret_cast<IProtocolMethod<Side::CLIENT>*>(&this->_protocol));
//         }

//         void    attachToConnection(IClientConnection* endpoint) override
//         {
//             connection_type* ep = dynamic_cast<connection_type*>(endpoint);
//             if (ep == nullptr)
//                 return ;
//             this->connections.push_back(ep);
//         }

//         void    detachFromConnection(const int socket) override
//         {
//             for (std::vector<connection_type*>::iterator it = this->connections.begin(); it != this->connections.end(); ++it)
//             {
//                 if ((*it)->getSocket() == socket)
//                 {
//                     this->connections.erase(it);
//                     return ;
//                 }
//             }
//         }

//     public:
//         std::vector<connection_type*>   connections;

//     private:
//         protocol_type   _protocol;
// };




// template<Side S, class Proto>
// class GatewayInterface
// {
//     typedef PacketParser<Proto>			        parser_type;
//     typedef GatewayInterface<S, Proto>	        interface_type;

//     typedef ProtocolMethod<S, Proto>            protocol_type;
//     typedef protocol_type::data_type            protocol_data_type;
//     typedef ServerClient<protocol_data_type>    client_type;
// };

