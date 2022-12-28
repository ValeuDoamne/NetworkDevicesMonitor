#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include "network/client.h"

#include "log_config.h"

#include "file_handler.h"

#include "agent_comm.h"


void logger(const Config& configuration)
{
	std::cout << "Connecting to: " << configuration.agent_socket_file << std::endl;

	net::client client{configuration.agent_socket_file, false};
	client.connect();
	
	send_create_table_message(client, configuration.table_name, configuration.create_table);
	std::vector<std::string> last_lines(configuration.files.size(), "");


	auto event_handler = [&client, &configuration](const std::vector<std::string>& files, const inotify_event& event)
	{
		std::string filename = "";

		if(event.len > 0)
		{
			filename = files[event.wd-1] + "/" + event.name; 
		} else {
			filename = files[event.wd-1];
		}
		auto last_line = get_last_line(filename);
		escape_quote(last_line);
		send_insert_table(client, configuration.table_name, last_line, filename);
	};
	
	event_loop(configuration.files, configuration.timeout, event_handler);
	
	client.close_connection();
}


int main(int argc, char **argv)
{
	std::string config_path = "./logger.toml";
	Config configuration;
	parse_command_line_arguments(argc, argv, config_path);
	parse_configuration(configuration, config_path); 

	logger(configuration);
	return 0;
}
