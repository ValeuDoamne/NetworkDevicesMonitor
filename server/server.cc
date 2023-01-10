#include <iostream>
#include <semaphore>
#include <json/json.h>
#include <future>
#include <sys/wait.h>
#include <cstdarg>
#include <chrono>

#include <pqxx/pqxx>
#include "headers/server_config.h"

#include "components/agent_server/agent_server.h"
#include "components/desktop_server/desktop_server.h"


int main(int argc, char **argv)
{
	Config configuration;
	std::string configuration_path = "./server_config.toml";
	parse_command_line_arguments(argc, argv, configuration_path);
	parse_configuration(configuration, configuration_path);
	
	pid_t child = fork();
	if(child == 0) {
		desktop_server(configuration);
	} else if(child == -1)
	{
		perror("Error in creating the desktop clients server");
	} else {
		server(configuration);
	}
		
	return 0;
}
