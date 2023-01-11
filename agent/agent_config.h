#pragma once
#include "toml.hpp"

struct Config {
	/* Agent Setup */
	std::string agent_socket_file;
	uint32_t    number_of_connections;
		

	/* Connection with server */
	std::string server_host;
	uint16_t    server_port;
	bool        server_secure_connection;
};

void parse_configuration(Config& configuration, const std::string& config_path)
{
	toml::table tbl;
	try {
		tbl = toml::parse_file(config_path);	

		configuration.agent_socket_file		= tbl["agent"]["socket_file"].value_or("/run/agentd.pid");
		configuration.number_of_connections     = tbl["agent"]["number_of_connections"].value_or(10);

	
		configuration.server_host               = tbl["server"]["hostname"].value_or("localhost");
		configuration.server_port		= tbl["server"]["port"].value_or(8888);
		configuration.server_secure_connection  = tbl["server"]["secure"].value_or(true);
	
	} catch (const toml::parse_error& err)
	{
		std::cerr << "[\033[31mERROR\033[0m]: Error parsing file '" << *err.source().path << "':\n" << err.description() << "\n  (" << err.source().begin << ")\n";
	}
}

void show_help();

void parse_command_line_arguments(int argc, char **argv, std::string& config_path) {
	for(int i = 1; i < argc; i++)
	{
		if(strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
			show_help();
		}
		if(strcmp(argv[i], "--config") == 0 || strcmp(argv[i], "-c") == 0) {
			if(i+1 == argc)	show_help();
			config_path = argv[i+1];	
		}
	}
}

void show_help()
{
	printf("Help for the agent utility:\n");
	printf("\t--config -c: [path]\t Set the server configuration path, default: ./server.toml\n");
	printf("\t--help   -h:\t\t Show this help.\n");
	_exit(0);
}

