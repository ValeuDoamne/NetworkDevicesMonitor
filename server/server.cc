#include <iostream>
#include <semaphore>
#include <json/json.h>
#include <future>
#include <sys/wait.h>

#include "network/network.h"
#include "network/server.h"

#include <pqxx/pqxx>

#include "server_config.h"


pqxx::connection *db_connection;

void create_table(const Json::Value& agent_value)
{
	auto table_name = agent_value["table_name"].as<std::string>();
	std::cout << table_name << std::endl;
	pqxx::work dbwork{*db_connection};
	std::string query = "CREATE TABLE " + dbwork.esc(table_name) + "(";
	
	auto array = agent_value["columns"];
	
	for(int i = 0; i < array.size(); i++)
	{
		auto column_name = array[i].as<std::string>();
		if(column_name.find(')') != std::string::npos)
		{
			std::cerr << "SQLi detected\n";
			return;
		}
		if(i != array.size()-1)
			query += dbwork.esc(column_name) + ", ";
		else 
			query += dbwork.esc(column_name) + ");"; 
	}
	
	std::cerr << "[DEBUG]: Query is: " << query << std::endl;
	dbwork.exec(query);
	dbwork.commit();

	std::cout << "[INFO]: " << "Table succesfully created: " << table_name << std::endl;
}

#include <cstdarg>

std::string format_query(const char *format, ...)
{
	char *buffer = nullptr;
	size_t buffer_size = 0;
	va_list ap;
	
	va_start(ap, format);
	buffer_size = vsnprintf(buffer, buffer_size, format, ap);
	va_end(ap);

	buffer = new char[buffer_size+1];
	va_start(ap, format);
	buffer_size = vsnprintf(buffer, buffer_size, format, ap);
	va_end(ap);
	
	std::string s = std::string(buffer);
	delete[] buffer;
	return s;
}

void insert_table(const Json::Value& agent_value, const std::string& host_str)
{
	static std::binary_semaphore critical_section{0};
	pqxx::work txn{*db_connection};
	auto table_name = agent_value["table_name"].as<std::string>();
	auto date       = agent_value["date"].as<std::string>();
	auto msg        = agent_value["msg"].as<std::string>();
	auto file       = agent_value["file"].as<std::string>();
	
	std::string query = format_query("INSERT INTO %s(date, file, host, msg) VALUES ('%s', '%s', '%s', '%s');", txn.esc(table_name).c_str(), txn.esc(date).c_str(), txn.esc(file).c_str(), txn.esc(host_str).c_str(), txn.esc(msg).c_str());

	std::cout << "Query: " << query << std::endl;

	txn.exec(query);
	txn.commit();
}

void process_json(const Json::Value& agent_value, const std::string& host_str)
{
	auto command = agent_value["cmd"].as<std::string>();
	if(command == "create_table")
	{
		create_table(agent_value);
	} else if(command == "insert_table")
	{
		insert_table(agent_value, host_str);
	}
}

void handle_agent(const std::string& thread_id, net::accepted_client& agent)
{
	while(agent.is_connected()) {
		auto json = agent.receive_message();
		std::cout << "Received from agent: " << json << std::endl;
		Json::Value agent_value;
		try{
			std::istringstream stream(json);
			stream >> agent_value;
		} catch (const std::exception& e)
		{
			std::cerr << "[Error]: " << e.what() << std::endl;
			continue; // not valid json
		}
		process_json(agent_value, agent.get_host());
	}
}


void server(const Config& configuration)
{
	db_connection = new pqxx::connection{"postgresql://"+configuration.username+":"+configuration.password+"@"+configuration.db_host+":"+std::to_string(configuration.db_port)+"/"+configuration.db_name};
	net::server agent_server(configuration.hostname, configuration.port, net::protocol::TCP, configuration.number_of_connections, configuration.server_secure_connection);
	if(configuration.server_secure_connection)
		agent_server.set_certificate_path(configuration.server_certificate, configuration.server_certificate_key);
	agent_server.listen();
	int thread_id = 0;
	while(true)
	{
		net::accepted_client agent = agent_server.accept_connection();
		auto t = std::async(handle_agent, "Thread #0", std::ref(agent));
	}
	agent_server.close_connection();
	
	db_connection->close();
	delete db_connection;
}


int main(int argc, char **argv)
{
	Config configuration;
	std::string configuration_path = "./server_config.toml";
	parse_command_line_arguments(argc, argv, configuration_path);
	parse_configuration(configuration, configuration_path);

	server(configuration);
	return 0;
}
