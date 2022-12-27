#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include "network/client.h"

#include "log_config.h"

void send_create_table_message(net::client& client)
{
	client.send_message(R"({"cmd": "create_table", "table_name" : "logs", "columns" : ["id INT PRIMARY KEY", "date DATE DEFAULT CURRENT_DATE", "host TEXT", "msg TEXT"] })");
}

void send_insert_table(net::client& client, const std::string& date, const std::string& msg, const std::string& file)
{
	char *buffer = new char[1024];
	snprintf(buffer, 1024, R"({"cmd":"insert_table", "table_name": "logs", "date": "%s", "msg":"%s", "file": "%s"})", date.c_str(), msg.c_str(), file.c_str()); 
	printf("%s", buffer);
	std::string s = std::string(buffer, strlen(buffer));
	delete[] buffer;
	client.send_message(s);
}


void escape_quote(std::string& str)
{
	size_t i = str.find('\"');
	while (i != std::string::npos)
	{
		std::string part1 = str.substr(0, i-1);
		std::string part2 = str.substr(i + 2);
		str = part1 + R"(\")" + part2; // Use "\\\\" instead of R"(\\)" if your compiler doesn't support C++11's raw string literals
		i = str.find('\"', i + 3);
	}
	i = str.find('\'');
	while (i != std::string::npos)
	{
		std::string part1 = str.substr(0, i-1);
		std::string part2 = str.substr(i + 2);
		str = part1 + R"(\")" + part2; // Use "\\\\" instead of R"(\\)" if your compiler doesn't support C++11's raw string literals
		i = str.find('\'', i + 3);
	}
}

int main(int argc, char **argv)
{
	std::string config_path = "./logger.toml";
	Config configuration;
	parse_command_line_arguments(argc, argv, config_path);
	parse_configuration(configuration, config_path); 
	std::cout << "Connecting to: " << configuration.agent_socket_file << std::endl;

	net::client client{"/tmp/agent.pid", false};
	client.connect();
	
	std::vector<std::ifstream> watched_files;
	for(auto& file : configuration.files)
	{
		watched_files.push_back(std::ifstream{file});
	}

	while(true)
	{
		for(int i = 0; i < watched_files.size(); i++)
		{
			std::string date = "2022 ", msg;
			std::string line;
			std::getline(watched_files[i], line);
			std::istringstream sstream(line);
			
			std::string word;
			int word_count = 0;
			while(sstream >> word)
			{
				if(word_count++ < 3) date += word+" ";
				else      msg += word+ " ";
			}
			escape_quote(msg);
			if(msg.length() == 0)
			{
				continue;
			}
			send_insert_table(client, date, msg, configuration.files[i]);
		}
		sleep(configuration.timeout);
	}
	client.close_connection();
	return 0;
}
