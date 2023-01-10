#pragma once
#include <stdio.h>
#include <string>
#include <cstring>
#include <json/json.h>

#include "network/client.h"

void send_create_table_message(net::client& client, const std::string& table_name, const std::string& columns_list)
{

	Json::Value root;
	root["cmd"]        = "create_table";
	root["table_name"] = table_name;
	try {
		std::istringstream stream(columns_list);
		stream >> root["columns"];	
	} catch (const std::exception& e)
	{
		throw e;	
	}

	std::string json_string = root.toStyledString();
	client.send_message(json_string);
}


void send_insert_table(net::client& client, const std::string& table_name, const std::string& msg, const std::string& file)
{
	Json::Value root;
	root["cmd"] = "insert_table";
	root["table_name"] = table_name;
	
	Json::Value column1, column2;
	column1["column_name"] = "message";
	column1["information"] = msg;
	column2["column_name"] = "file";
	column2["information"] = file;

	root["columns"][0] = column1;
	root["columns"][1] = column2;

	client.send_message(root.toStyledString());
}
