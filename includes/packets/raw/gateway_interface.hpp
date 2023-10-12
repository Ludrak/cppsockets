// #pragma once

// #include <string>
// #include "common/interfaces/gateway_interface.hpp"
// #include "common/protocols/protocol.hpp"
// #include "protocols/tcp/protocol.hpp"


// template<>
// class GatewayInterface<Side::CLIENT, Packets::RAW> : public IGatewayInterface<Side::CLIENT>
// {
//     public:
//         typedef ProtocolMethod<Protocol::TCP>                  protocol_type;
//         typedef PacketParser<Packets::RAW>                     parser_type;
//         typedef GatewayInterface<Side::CLIENT, Packets::RAW>   interface_type;

//         GatewayInterface(Client& client);

//         virtual void    onConnected() override;
//         virtual void    onDisconnected() override;

//         virtual void    onReceived(const std::string& message);

//         void            emit(const std::string& str);
    
//     protected:
//         // called by server, interfaces with public virtual members 
//         void            receive(void* parsed_data) override;
// };

// template<>
// class GatewayInterface<Side::SERVER, Packets::RAW> : public IGatewayInterface<Side::SERVER>
// {
//     public:
//         static constexpr Packets packet_sequencer_type = Packets::RAW;
//         typedef ProtocolMethod<Protocol::TCP>                  protocol_type;
//         typedef PacketParser<Packets::RAW>                     parser_type;
//         typedef GatewayInterface<Side::SERVER, Packets::RAW>   interface_type;
//         typedef interface_type::client_type                    client_type;

//         GatewayInterface(Server& server);

//         virtual void    onConnected(client_type& client) override;
//         virtual void    onDisconnected(client_type& client) override;

//         virtual void    onReceived(client_type& client, const std::string& message);

//         void            emit(client_type& client, const std::string& str);


//     protected:
//         // called by server, interfaces with public virtual members 
//         void            receive(client_type& from, void* parsed_data) override;
// };