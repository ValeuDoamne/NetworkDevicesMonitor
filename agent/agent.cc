#include <iostream>
#include <string>
#include <string_view>
#include <optional>
#include <json/json.h>


#include "network/network.h"
#include "network/local_server.h"
#include "network/server.h"
#include "network/client.h"


#include "agent_config.h"


void agent(const Config& configuration)
{
	try {
		net::local_server agent_server(configuration.agent_socket_file, configuration.number_of_connections, false);
		net::client       agent_client(configuration.server_host, configuration.server_port, net::protocol::TCP, configuration.server_secure_connection);
		
		std::cout << "Connecting to: " << configuration.server_host << ":" << configuration.server_port << std::endl; 
	
		agent_server.listen();
		agent_client.connect();

		agent_server.select(
			[&agent_client](net::accepted_client& client) {
				auto json_message = client.receive_message();
				std::cerr << "[DEBUGS]: " << json_message << std::endl;
				if(client.is_connected())
					agent_client.send_message(json_message);
			}, 1);
		
		agent_client.close_connection();
		agent_server.close_connection();

	} catch (const net::network_error& e)
	{
		std::cerr << "[\033[31mERROR\033[0m]: " << e.what() << std::endl; 
	}
	
}


int main(int argc, char **argv)
{
	std::string config_path = "./agent_config.toml";
	Config configuration;
	
	parse_command_line_arguments(argc, argv, config_path);
	parse_configuration(configuration, config_path);
	agent(configuration);

	return 0;
}
