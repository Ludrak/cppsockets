#include "client.hpp"
#include "protocols/tcp/interface.hpp"

class ClientInterface : public GatewayInterface<Side::CLIENT, Protocols::Transport::TCP>
{
    public:
    ClientInterface(Client& client)
    : GatewayInterface(client)
    {}

    void onReceived(connection_type* connection, const std::string& message) override
    {
        (void) connection;
        std::cout << "Server sent : '" << message << "'" << std::endl;
       // this->emit("Hey !");

    }
};

int main()
{
    Client *client = new Client();

    // 1. connect to single endpoint
    client->connect<ClientInterface>("localhost", 8080, AF_INET);

    // 2. connect single interface to multiple endpoints
    // ClientInterface interface = ClientInterface(*client);
    // client->connect(interface, "localhost", 8080);
    // client->connect(interface, "localhost", 6060);

    // 3. connect multiple instances of interface for multiple endpoints
    // client->connect<ClientInterface>("localhost", 8080, AF_INET);
    // client->connect<ClientInterface>("localhost", 6060, AF_INET);


    while (client->wait_update())
    {
        std::cout << "*** loop ***" << std::endl;
    }

    delete client;
}
