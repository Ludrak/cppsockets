#pragma once

#include <string>
#include "common/protocols/protocol.hpp"
#include "protocol_parser.hpp"

template<>
class GatewayInterface<Side::CLIENT, Protocol::RAW> : public GatewayInterfaceBase<Side::CLIENT>
{

    public:
        typedef ProtocolParser<Protocol::RAW>                   parser_type;
        typedef GatewayInterface<Side::CLIENT, Protocol::RAW>   interface_type;

        GatewayInterface()
        : GatewayInterfaceBase(reinterpret_cast<ProtocolParserBase*>(new parser_type())) {}

        virtual void    onConnected() override {};
        virtual void    onDisconnected() override {};

        virtual void    onReceived(const std::string& message) = 0;
    
    protected:
        // called by server, interfaces with public virtual members 
        void            receive(void* parsed_data) override
        {
            this->onReceived(std::string(reinterpret_cast<char*>(parsed_data)));
        }
};

template<>
class GatewayInterface<Side::SERVER, Protocol::RAW> : public GatewayInterfaceBase<Side::SERVER>
{
    public:
        typedef ProtocolParser<Protocol::RAW>                   parser_type;
        typedef GatewayInterface<Side::SERVER, Protocol::RAW>   interface_type;

        GatewayInterface(Server& server, ServerEndpoint& endpoint)
        : GatewayInterfaceBase(server, endpoint, reinterpret_cast<ProtocolParserBase*>(new parser_type())) {}

        virtual void    onConnected(const ServerClient& client) override { (void)client; };
        virtual void    onDisconnected(const ServerClient& client) override { (void)client; };

        virtual void    onReceived(const ServerClient& client, const std::string& message) = 0;

    protected:
        // called by server, interfaces with public virtual members 
        void            receive(const ServerClient& from, void* parsed_data) override
        {
            std::string *data = reinterpret_cast<std::string*>(parsed_data);
            this->onReceived(from, *data);
            delete data;
        }
};