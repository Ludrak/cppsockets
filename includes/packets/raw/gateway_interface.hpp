#pragma once

#include <string>
#include "common/interfaces/gateway_interface.hpp"
#include "packets/raw/gateway_interface.hpp"
#include "common/protocols/protocol.hpp"
#include "protocols/tcp/protocol.hpp"

template<>
class GatewayInterface<Side::CLIENT, Packets::RAW> : public GatewayInterfaceBase<Side::CLIENT>
{
    public:
        typedef ProtocolMethod<Protocol::TCP>                  protocol_type;
        typedef PacketParser<Packets::RAW>                     parser_type;
        typedef GatewayInterface<Side::CLIENT, Packets::RAW>   interface_type;

        GatewayInterface(Client& client, ClientConnection& connection);

        virtual void    onConnected() override;
        virtual void    onDisconnected() override;

        virtual void    onReceived(const std::string& message);

        void            emit(const std::string& str);
    
    protected:
        // called by server, interfaces with public virtual members 
        void            receive(void* parsed_data) override;
};

template<>
class GatewayInterface<Side::SERVER, Packets::RAW> : public GatewayInterfaceBase<Side::SERVER>
{
    public:
        typedef ProtocolMethod<Protocol::TCP>                  protocol_type;
        typedef PacketParser<Packets::RAW>                     parser_type;
        typedef GatewayInterface<Side::SERVER, Packets::RAW>   interface_type;

        GatewayInterface(Server& server, ServerEndpoint& endpoint);

        virtual void    onConnected(ServerClient& client) override;
        virtual void    onDisconnected(ServerClient& client) override;

        virtual void    onReceived(ServerClient& client, const std::string& message);

        void            emit(ServerClient& client, const std::string& str);

    protected:
        // called by server, interfaces with public virtual members 
        void            receive(ServerClient& from, void* parsed_data) override;
};