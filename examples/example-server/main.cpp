# include "cpp_argparse.hpp"

# define DISABLE_WARNINGS
# define USE_SELECT

# include "server.hpp"
# include "packets/raw/gateway_interface.hpp"


class ExampleInterface : public GatewayInterface<Side::SERVER, Packets::RAW>
{
	public:
		ExampleInterface(Server& server, ServerEndpoint& endpoint)
		: GatewayInterface(server, endpoint)
		{

		}

		void onReceived(ServerClient& client, const std::string& message) override
		{
			std::cout << "Received from " << client.getHostname() << " : " << '\'' << message << '\'' << std::endl;
			this->emit(client, "Received: '" + message + "'\n");
		}

		void onConnected(ServerClient& client) override
		{
			std::cout << "Client connected on endpoint " << this->endpoint.getHostname() << " from " << client.getHostname() << std::endl;
			this->emit(client, "Hi\n");
		}

		void onDisconnected(ServerClient& client) override
		{
			std::cout << "Client disconnected from endpoint " << this->endpoint.getHostname() << " from " << client.getHostname() << std::endl;
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
	server->addEndpoint<ExampleInterface>(serverIp, serverPort, AF_INET);
	server->addEndpoint<ExampleInterface>(serverIp, serverPort + 1010, AF_INET);

	server->start_listening();
	while (server->wait_update())
	{
		std::cout << "Server loop" << std::endl;
	}
	

	delete server;
	return (EXIT_SUCCESS);
}