# include "cpp_argparse.hpp"

// #ifdef __linux__
# define DISABLE_WARNINGS
# define USE_SELECT
// //todo # define USE_EPOLL
// #elif defined(__APPLE__)
// # define USE_POLL
// #endif

// # define ENABLE_TLS

# include "server.hpp"

# include "protocols/raw/protocol_interface.hpp"
# include "protocols/raw/protocol_parser.hpp"

class ExampleInterface : public GatewayInterface<Side::SERVER, Protocol::RAW>
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


	// Server Router
	// std::list<ServerEndpoint> endpoints;
	// endpoints.emplace_back(make_endpoint<ExampleInterface>("localhost", 8080, false, AF_INET));
	// endpoints.emplace_back(make_endpoint<ExampleRawInterface>("localhost", 4040, true, AF_INET));
	// endpoints.emplace_back(make_endpoint<ExampleRawInterface>("localhost", 6060, false, AF_INET6));
	// Server	*server = new Server(endpoints);


	// 1. constructor endpoint
	//Server	*server = new Server<ExampleInterface>(serverIp, serverPort, AF_INET);

	// 2. addEndpoint endpoint
	Server	*server = new Server();
	server->addEndpoint<ExampleInterface>(std::string("localhost"), 8080, AF_INET);
	server->addEndpoint<ExampleInterface>(std::string("localhost"), 6060, AF_INET);

	// server->ssl_cert_file = 		"./certificates/cert.pem";
	// server->ssl_private_key_file =	"./certificates/cert.pem";
	
	server->start_listening();
	while (server->wait_update())
	{
		// do taskmaster things...
		std::cout << "Server loop" << std::endl;
	}
	
	// LOG_INFO(LOG_CATEGORY_INIT, "Exited properly.")

	delete server;
	return (EXIT_SUCCESS);
}