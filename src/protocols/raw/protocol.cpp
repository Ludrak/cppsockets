
#include <string>
#include "common/protocols/protocol.hpp"
#include "protocols/raw/protocol_interface.hpp"
#include "protocols/raw/protocol_parser.hpp"

#include "client.hpp"
#include "server.hpp"

/* PARSER */
ProtocolParserBase::EvalResult  ProtocolParser<Protocol::RAW>::eval(const char *data_strip, const size_t strip_size)
{
    (void)strip_size;
    (void)data_strip;
    return ProtocolParserBase::EvalResult::COMPLETE;
}

void*                           ProtocolParser<Protocol::RAW>::parse(const char *data_strip, const size_t strip_size)
{
    std::string *data = new std::string(data_strip, strip_size);
    return (reinterpret_cast<void*>(data));
}


/* CLIENT SIDE*/
GatewayInterface<Side::CLIENT, Protocol::RAW>::GatewayInterface(Client& client, ClientConnection& connection)
: GatewayInterfaceBase(client, connection, reinterpret_cast<ProtocolParserBase*>(new parser_type()))
{}

void            GatewayInterface<Side::CLIENT, Protocol::RAW>::onConnected() {}
void            GatewayInterface<Side::CLIENT, Protocol::RAW>::onDisconnected() {}
void            GatewayInterface<Side::CLIENT, Protocol::RAW>::onReceived(const std::string& message) { (void)message; }

void            GatewayInterface<Side::CLIENT, Protocol::RAW>::emit(const std::string& str)
{
    this->client.sendData(this->connection, str.c_str(), str.size());
}

void            GatewayInterface<Side::CLIENT, Protocol::RAW>::receive(void* parsed_data)
{
    std::string *data = reinterpret_cast<std::string*>(parsed_data);
    this->onReceived(*data);
    delete data;
}




/* SERVER SIDE*/
GatewayInterface<Side::SERVER, Protocol::RAW>::GatewayInterface(Server& server, ServerEndpoint& endpoint)
: GatewayInterfaceBase(server, endpoint, reinterpret_cast<ProtocolParserBase*>(new parser_type()))
{}

void            GatewayInterface<Side::SERVER, Protocol::RAW>::onConnected(ServerClient& client) { (void)client; };
void            GatewayInterface<Side::SERVER, Protocol::RAW>::onDisconnected(ServerClient& client) { (void)client; };
void            GatewayInterface<Side::SERVER, Protocol::RAW>::onReceived(ServerClient& client, const std::string& message) { (void)client; (void)message; };

void            GatewayInterface<Side::SERVER, Protocol::RAW>::emit(ServerClient& client, const std::string& str)
{
    this->server.sendData(client, str.c_str(), str.size());
}

void            GatewayInterface<Side::SERVER, Protocol::RAW>::receive(ServerClient& from, void* parsed_data)
{
    std::string *data = reinterpret_cast<std::string*>(parsed_data);
    this->onReceived(from, *data);
    delete data;
}
