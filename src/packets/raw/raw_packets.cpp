
#include <string>
#include "common/interfaces/gateway_interface.hpp"
#include "packets/raw/gateway_interface.hpp"
#include "packets/raw/packet_parser.hpp"

#include "client.hpp"
#include "server.hpp"

/* PARSER */
PacketParserBase::EvalResult  PacketParser<Packets::RAW>::eval(const char *data_strip, const size_t strip_size)
{
    (void)strip_size;
    (void)data_strip;
    return PacketParserBase::EvalResult::COMPLETE;
}

void*                           PacketParser<Packets::RAW>::parse(const char *data_strip, const size_t strip_size)
{
    std::string *data = new std::string(data_strip, strip_size);
    return (reinterpret_cast<void*>(data));
}


/* CLIENT SIDE*/
GatewayInterface<Side::CLIENT, Packets::RAW>::GatewayInterface(Client& client)
: GatewayInterfaceBase(client, reinterpret_cast<PacketParserBase*>(new parser_type()), reinterpret_cast<ProtocolBase*>(new protocol_type()))
{}

void            GatewayInterface<Side::CLIENT, Packets::RAW>::onConnected() {}
void            GatewayInterface<Side::CLIENT, Packets::RAW>::onDisconnected() {}
void            GatewayInterface<Side::CLIENT, Packets::RAW>::onReceived(const std::string& message) { (void)message; }

void            GatewayInterface<Side::CLIENT, Packets::RAW>::emit(const std::string& str)
{
    for (ClientConnection* connection : this->connection)
        if (connection)
            this->client.sendData(*connection, str.c_str(), str.size());
}

void            GatewayInterface<Side::CLIENT, Packets::RAW>::receive(void* parsed_data)
{
    std::string *data = reinterpret_cast<std::string*>(parsed_data);
    this->onReceived(*data);
    delete data;
}




/* SERVER SIDE*/
GatewayInterface<Side::SERVER, Packets::RAW>::GatewayInterface(Server& server)
: GatewayInterfaceBase(server, reinterpret_cast<PacketParserBase*>(new parser_type()), reinterpret_cast<ProtocolBase*>(new protocol_type()))
{}

void            GatewayInterface<Side::SERVER, Packets::RAW>::onConnected(ServerClient& client) { (void)client; };
void            GatewayInterface<Side::SERVER, Packets::RAW>::onDisconnected(ServerClient& client) { (void)client; };
void            GatewayInterface<Side::SERVER, Packets::RAW>::onReceived(ServerClient& client, const std::string& message) { (void)client; (void)message; };

void            GatewayInterface<Side::SERVER, Packets::RAW>::emit(ServerClient& client, const std::string& str)
{
    this->server.sendData(client, str.c_str(), str.size());
}

void            GatewayInterface<Side::SERVER, Packets::RAW>::receive(ServerClient& from, void* parsed_data)
{
    std::string *data = reinterpret_cast<std::string*>(parsed_data);
    this->onReceived(from, *data);
    delete data;
}
