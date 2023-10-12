#include "common/interfaces/gateway_interface.hpp"
#include "protocols/tcp/protocol.hpp"

template<>
class GatewayInterface<Side::SERVER, Protocols::Transport::TCP> : public IGatewayInterface<Side::SERVER>
{
    public:
       //typedef PacketParser<Protocols::Transport::TCP>			            parser_type;
        typedef GatewayInterface<Side::SERVER, Protocols::Transport::TCP>   interface_type;

        typedef Protocols::Transport::TCP                                   protocol_type;
        typedef ProtocolMethod<Side::SERVER, Protocols::Transport::TCP>     protocol_method_type;
        typedef protocol_method_type::data_type                             protocol_data_type;

        typedef ServerClient<protocol_data_type>                            client_type;
        typedef ServerEndpoint<Protocols::Transport::TCP>                   endpoint_type;

        GatewayInterface(Server& server);

        IProtocolMethod<Side::SERVER>*     getProtocol() override;

        void    attachToEndpoint(IServerEndpoint* endpoint) override;
        void    detachFromEndpoint(const int socket) override;

        // Basic hooks called by server, redirect with correct type
        void            onConnected(IServerClient* client) override;
        virtual void    onConnected(client_type* client) { (void) client; }

        void            onDisconnected(IServerClient* client) override;
        virtual void    onDisconnected(client_type* client) { (void) client; }
       
        // Hooks called by ProtocolMethod<Protocols::Transport::TCP>
        virtual void    onReceived(client_type* client, const std::string& tcp_message) { (void)client; (void)tcp_message; }


    public: 
        Server&                     server;
        std::vector<endpoint_type*> endpoints;

    private:
        protocol_method_type        _protocol;
};

template<>
class GatewayInterface<Side::CLIENT, Protocols::Transport::TCP> : public IGatewayInterface<Side::CLIENT>
{
    public:
        typedef GatewayInterface<Side::CLIENT, Protocols::Transport::TCP>   interface_type;

        typedef Protocols::Transport::TCP                                   protocol_type;
        typedef ProtocolMethod<Side::CLIENT, Protocols::Transport::TCP>     protocol_method_type;
        typedef protocol_method_type::data_type                             protocol_data_type;
        
        typedef ClientConnection<protocol_type, protocol_data_type>         connection_type;

        GatewayInterface(Client& client);

        IProtocolMethod<Side::CLIENT>*     getProtocol() override;

        void    attachToConnection(IClientConnection* endpoint) override;
        void    detachFromConnection(const int socket) override;

        // Basic hooks called by server, redirect with correct type
        void            onConnected(IClientConnection* connection) override;
        virtual void    onConnected(connection_type* connection) { (void) connection; }

        void            onDisconnected(IClientConnection* connection) override;
        virtual void    onDisconnected(connection_type* connection) { (void) connection; }

        // Hooks called by ProtocolMethod<Protocols::Transport
        virtual void    onReceived(connection_type* client, const std::string& tcp_message) { (void) client;  (void) tcp_message; }


    public:
        Client&                         client;
        std::vector<connection_type*>   connections;

    private:
        protocol_type   _protocol;
};
