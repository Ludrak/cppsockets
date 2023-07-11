# include "cpp_argparse.hpp"

#ifdef __linux__
# define DISABLE_WARNINGS
# define USE_SELECT
//todo # define USE_EPOLL
#elif defined(__APPLE__)
# define USE_POLL
#endif

# define ENABLE_TLS
# include "net/server.hpp"
# include "dto.hpp"

class ClientData
{
	public:
		// independant data storage for every client
		bool	asked_status = false;
};

class TaskmasterClientsManager : public ServerClientsHandler<TaskmasterClientsManager, ClientData>
{
    public:
        TaskmasterClientsManager(server_type& server)
        : handler_type(server)
        {}

		void onConnected(client_type& client)
		{
			if (client.useTLS())
				std::cout << client.getCertificate() << std::endl;
		}

        void declareMessages()
        {
            this->server.onMessage("status", 
                server_type::make_handler<StatusDTO>([](server_type& server, client_type& client, DTO* dto)
                {
                    // always safe
					StatusDTO* status = reinterpret_cast<StatusDTO*>(dto);
					
					client.asked_status = status->value;
					LOG_INFO(LOG_CATEGORY_NETWORK, "Received status with value of " << status->value << ", sending back value 42");

					// resending back packet with different value
					status->value = 42;
					server.emit_now("status_response", *status, client);

					::close(client.getSocket());
				}
            ));
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
		ARG_USAGE(" == Server ===");
		return (EXIT_SUCCESS);
	}

	std::list<ServerEndpoint> endpoints;
	endpoints.emplace_back(ServerEndpoint("localhost", 8080, false, AF_INET));
	endpoints.emplace_back(ServerEndpoint("localhost", 4040, true, AF_INET));
	endpoints.emplace_back(ServerEndpoint("localhost", 6060, false, AF_INET6));
	Server<TaskmasterClientsManager>	*server = new Server<TaskmasterClientsManager>(endpoints);
	server->ssl_cert_file = 		"./certificates/cert.pem";
	server->ssl_private_key_file =	"./certificates/cert.pem";
	
	server->start_listening();
	do
	{
		// do taskmaster things...
		std::cout << "Server loop" << std::endl;
	}
	while (server->wait_update());

	LOG_INFO(LOG_CATEGORY_INIT, "Exited properly.")

	return (EXIT_SUCCESS);
}