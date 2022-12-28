#pragma once
#include <stdio.h>
#include <string>
#include <cstring>

#include "network/client.h"

void send_create_table_message(net::client& client, const std::string& table_name, const std::string& columns_list)
{

	char *buffer = new char[1024];
	snprintf(buffer, 1024, R"({"cmd":"create_table", "table_name": "%s", "columns": %s})", table_name.c_str(), columns_list.c_str()); 

	printf("[DEBUG]: Query: %s\n", buffer);

	std::string s = std::string(buffer, strlen(buffer));
	delete[] buffer;
	client.send_message(s);
}

void send_insert_table(net::client& client, const std::string& table_name, const std::string& msg, const std::string& file)
{
	char *buffer = new char[1024];
	snprintf(buffer, 1024, R"({"cmd":"insert_table", "table_name": "%s", "columns" : [{"column_name": "message", "information" : "%s"}, {"column_name" : "file", "information": "%s"}]})", table_name.c_str(), msg.c_str(), file.c_str()); 
	
	printf("[DEBUG]: Query: %s\n", buffer);

	std::string s = std::string(buffer, strlen(buffer));
	delete[] buffer;
	client.send_message(s);
}

void escape_quote(std::string& str)
{
	size_t i = str.find('\"');
	while (i != std::string::npos)
	{
		std::string part1 = str.substr(0, i);
		std::string part2 = str.substr(i + 1);
		str = part1 + R"(\")" + part2;
		i = str.find('\"', i + 2);
	}
	i = str.find('\'');
	while (i != std::string::npos)
	{
		std::string part1 = str.substr(0, i);
		std::string part2 = str.substr(i + 1);
		str = part1 + R"(\')" + part2; 
		i = str.find('\'', i + 2);
	}
}

