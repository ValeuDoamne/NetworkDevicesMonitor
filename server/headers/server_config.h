#pragma once

#include <stdlib.h>
#include <iostream>

#include "toml.hpp"
#include "server_config_struct.h"

void parse_configuration(Config& configuration, const std::string& config_file)
{
	toml::table tbl;
	try {
		tbl = toml::parse_file(config_file);

		configuration.hostname                  = tbl["agent_server"]["hostname"].value_or("0.0.0.0");
		configuration.port                      = tbl["agent_server"]["port"].value_or(8888); 
		configuration.server_secure_connection  = tbl["agent_server"]["secure"].value_or(true);
		configuration.server_certificate      	= tbl["agent_server"]["certificate_file"].value_or("");
		configuration.server_certificate_key	= tbl["agent_server"]["certificate_key_file"].value_or("");
		configuration.number_of_connections     = tbl["agent_server"]["number_of_connections"].value_or(10);
		
		configuration.clients_hostname                  = tbl["client_server"]["hostname"].value_or("0.0.0.0");
		configuration.clients_port                      = tbl["client_server"]["port"].value_or(8888); 
		configuration.clients_server_secure_connection  = tbl["client_server"]["secure"].value_or(true);
		configuration.clients_server_certificate      	= tbl["client_server"]["certificate_file"].value_or("");
		configuration.clients_server_certificate_key	= tbl["client_server"]["certificate_key_file"].value_or("");
		configuration.clients_number_of_connections     = tbl["client_server"]["number_of_connections"].value_or(10);

	
		configuration.db_host               = tbl["database"]["hostname"].value_or("localhost");
		configuration.db_port		    = tbl["database"]["port"].value_or(5432);
		configuration.username		    = tbl["database"]["username"].value_or("");
		configuration.password		    = tbl["database"]["password"].value_or("");
		configuration.db_name               = tbl["database"]["db_name"].value_or("");
		configuration.db_users              = tbl["database"]["db_users"].value_or("");
	
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
	printf("Help for the server utility:\n");
	printf("\t--config -c: [path]\t Set the server configuration path, default: ./server.toml\n");
	printf("\t--help   -h:\t\t Show this help.\n");
	_exit(0);
}
