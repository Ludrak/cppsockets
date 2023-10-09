#include "client.hpp"
#include "packets/raw/gateway_interface.hpp"

class ClientInterface : public GatewayInterface<Side::CLIENT, Packets::RAW>
{
    public:
    ClientInterface(Client& client, ClientConnection& connection)
    : GatewayInterface(client, connection)
    {}

    void onReceived(const std::string& message)
    {
        std::cout << "Server sent : '" << message << "'" << std::endl;
        this->emit("Hey !");
    }
};

int main()
{
    Client *client = new Client();

    client->connect<ClientInterface>("localhost", 8080, AF_INET);
    while (client->wait_update())
    {

    }

    delete client;
}
