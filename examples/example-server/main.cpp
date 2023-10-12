# include "cpp_argparse.hpp"

# define DISABLE_WARNINGS
# define USE_SELECT

# include "server.hpp"
# include "protocols/tcp/interface.hpp"


# include "common/data/data.hpp"


// class ClientDataExample
// {
// 	int a;
// 	int b;
// 	int c;
// };


class ExampleInterface : public GatewayInterface<Side::SERVER, Protocols::Transport::TCP>
{
		// typedef ServerClient<client_data_type, Data<> > client_type;
	public:
		ExampleInterface(Server& server)
		: GatewayInterface(server)
		{
		}

		void onReceived(client_type* client, const std::string& message) override
		{
			std::cout << "Received from " << client->getHostname() << " : " << '\'' << message << '\'' << std::endl;
			
			// this->emit(client, "Received: '" + message + "'\n");
			//client->a = 10;
		}

		void onConnected(client_type* client) override
		{
			std::cout << "on USER SPACE " << client->getEndpoint() << std::endl;

			std::cout << "Client connected on endpoint " << client->getEndpoint()->getHostname() << " from " << client->getHostname() << std::endl;
			// this->emit(client, "Hi\n");

			//client->getAddress4();

			//client->data
		}

		void onDisconnected(client_type* client) override
		{
			std::cout << "Client disconnected from endpoint " << client->getEndpoint()->getHostname() << " from " << client->getHostname() << std::endl;
		}
};


int main(int ac, char** av)
{
	bool		help = false;

	std::string	serverIp;
	int			serverPort;

	ARG_INIT(
		ARG_GROUP("server", "Server",
			ARG<std::string>("-i", "--ip", "ip address of taskmaster server", &serverIp, ARG_REQUIRED),
			ARG<int>("-p", "--port", "port of taskmaster server", &serverPort, ARG_REQUIRED)
		),
		ARG_NOVALUE("-h", "--help", "shows this usage", &help)
	);


	int parsing_failed = ARG_PARSE(ac, av);
	if (parsing_failed)
	{
		ARG_USAGE(" === Server ===");
		return (EXIT_FAILURE);
	}
	if (help)
	{
		ARG_USAGE(" === Server ===");
		return (EXIT_SUCCESS);
	}


	// 1. With a list of endpoints
	// std::list<ServerEndpoint> endpoints;
	// endpoints.emplace_back(make_endpoint<ExampleInterface>("localhost", 8080, false, AF_INET));
	// endpoints.emplace_back(make_endpoint<ExampleRawInterface>("localhost", 4040, true, AF_INET));
	// endpoints.emplace_back(make_endpoint<ExampleRawInterface>("localhost", 6060, false, AF_INET6));
	// Server	*server = new Server(endpoints);

	// 2. addEndpoint endpoint
	Server	*server = new Server();

	// Adding endpoints

	// option 1 : Add endpoint and instantiate the interface at the same time
	server->addEndpoint<ExampleInterface>(serverIp, serverPort, AF_INET);
	server->addEndpoint<ExampleInterface>(serverIp, serverPort + 1010, AF_INET);

	// option 2 : 
	// 1 - create interface
	ExampleInterface interface = ExampleInterface(*server);
	// 2- attach interface to multiple endpoints
	//    (all endpoints data is received, parsed, sent by a single interface, the clients however are split across all the endpoints and not shared in a single list)
	server->addEndpoint(interface, "localhost", 9000);
	server->addEndpoint(interface, "localhost", 8000);

	std::cout << "added endpoints" << std::endl;

	// server->start_listening();
	while (server->wait_update())
	{
		std::cout << "*** loop ***" << std::endl;
	}

	std::cout << "wait_update() returned false" << std::endl;
	
	// ExampleInterface it = ExampleInterface(*server);
	// ServerEndpoint   ep = ServerEndpoint(&it, "localhost", 10000);
	// InetAddress addr = InetAddress("localhost", 10000);
	// ServerClient     client = ServerClient(ep, ServerClient::createData<MyClientData>, 3, addr);
	

	delete server;
	return (EXIT_SUCCESS);
}