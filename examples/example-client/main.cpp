// #include <iostream>
// #include "cpp_argparse.hpp"


// #ifdef __linux__
// # define DISABLE_WARNINGS
// # define USE_SELECT
// //todo # define USE_EPOLL
// #elif defined(__APPLE__)
// # define USE_POLL
// #endif

// # define ENABLE_TLS
// #include "client.hpp"
// #include "dto.hpp"

// #include "common/packet/protocol.hpp"

// class ClientData
// {
//     // data storage for client to use in handlers
//     // (client context)
// };

// class TaskmasterClientHandler : public ClientHandler<TaskmasterClientHandler, ClientData>,
//                                 public MessagesServerProtocolInterface
// {
//     public:
//         TaskmasterClientHandler(client_type& Client)
//         : handler_type(Client)
//         {
//         }

//         void onConnected()
//         {
//             StatusDTO dto;
//             dto.value = 101;
//             this->client.emit("status", dto);
//         }

//         void declareMessages()
//         {
//             // this->client.onMessage<StatusDTO>("status_response", [](client_type& client, DTO* dto)
//             //     {
//             //         StatusDTO *status = reinterpret_cast<StatusDTO*>(dto);
//             //         LOG_INFO(LOG_CATEGORY_NETWORK, "Received status response from server with value: " << status->value);
//             //         client.disconnect();
//             //     }
//             // );

//             this->onMessage<StatusDTO>("status_response", [](client_type& client, DTO* dto)
//                 {
//                     StatusDTO *status = reinterpret_cast<StatusDTO*>(dto);
//                     LOG_INFO(LOG_CATEGORY_NETWORK, "Received status response from server with value: " << status->value);
//                     client.disconnect();
//                 }
//             );
//         }
// };


// int main(int ac, char **av)
// {
//     bool        help = false;

//     std::string serverIp;
//     int         serverPort;

//     bool        tls = false;

// 	ARG_INIT(
// 		ARG_GROUP("server", "Daemonized server running taskmaster",
// 			ARG<std::string>("-i", "--ip", "ip address of taskmaster server", &serverIp, ARG_REQUIRED),
// 			ARG<int>("-p", "--port", "port of taskmaster server", &serverPort, ARG_REQUIRED),
// 			ARG_NOVALUE("-t", "--tls", "use tls for connection", &tls)
// 		),
// 		ARG_NOVALUE("-h", "--help", "shows this usage", &help)
// 	);


// 	int parsing_failed = ARG_PARSE(ac, av);
// 	if (parsing_failed)
// 	{
// 		ARG_USAGE("  === Client === ");
// 		return (EXIT_FAILURE);
// 	}
// 	if (help)
// 	{
// 		ARG_USAGE("  === Client ===");
// 		return (EXIT_SUCCESS);
// 	}


//     Client<TaskmasterClientHandler> *client = new Client<TaskmasterClientHandler>(serverIp, serverPort);
//     if (tls)
//         client->enableTLS();
//     client->connect();

//     do
//     {
//         std::cout << "Client loop" << std::endl;
//     } while (client->wait_update());
    
//     delete client;

//     return (0);
// }



#include "client.hpp"
#include "protocols/raw/protocol_interface.hpp"

class ClientInterface : public GatewayInterface<Side::CLIENT, Protocol::RAW>
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
