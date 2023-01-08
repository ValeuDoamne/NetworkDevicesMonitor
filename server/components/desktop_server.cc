
#include "desktop_server.h"

#include "desktop_commands.h"

#include <future>
#include <vector>
#include <iostream>

#include <json/json.h>

static pqxx::connection *db_connection;
static pqxx::connection *db_connection_users;

std::string handle_command(const Json::Value& root)
{
	static CommandManager manager;
	Json::Value result;
	manager.add_command((Command *)(new LogIn(db_connection_users)));
	manager.add_command((Command *)(new LogOut));
	manager.add_command((Command *)(new GetData(db_connection)));

	auto received_command = root["command"].as<std::string>();
	for(const auto& command : manager.get_commands())
	{
		std::cerr << "[Debug]: Trying command " << command->name() << std::endl;
		if(command->name() == received_command)
		{
			std::cerr << "[Debug]: Command is executed " << command->name() << std::endl;
			result = command->command(root);
			break;
		}
	}
	return result.toStyledString();
}

void handle_client(net::accepted_client& client)
{
	while(client.is_connected())
	{
		auto json = client.receive_message();
		std::cerr << "[Debug]: Receive from client: " << json << std::endl;
		Json::Value root;
		try {
			std::istringstream stream(json);
			stream >> root;
		} catch (const std::exception& e) {
			std::cerr << "[ERROR]: Desktop clients server received bad json input" << std::endl;
		}

		auto send_back = handle_command(root);
		std::cerr << "[Debug]: Sending back to client: " << send_back << std::endl;
		client.send_message(send_back);
	}
	client.close_connection();
}


void setup_db_commands(const Config& configuration)
{
	db_connection = new pqxx::connection{"postgresql://"+configuration.username+":"+configuration.password+"@"+configuration.db_host+":"+std::to_string(configuration.db_port)+"/"+configuration.db_name};
	db_connection_users = new pqxx::connection{"postgresql://"+configuration.username+":"+configuration.password+"@"+configuration.db_host+":"+std::to_string(configuration.db_port)+"/"+configuration.db_users};

	db_connection->prepare("get_tables", "SELECT tablename FROM pg_catalog.pg_tables WHERE schemaname = 'public';");
	
	db_connection_users->prepare("login", "SELECT * FROM users WHERE username=$1 AND password=encode(digest($2, 'sha512'), 'hex');");

}

void desktop_server(const Config& configuration)
{
	setup_db_commands(configuration);

	std::cout << "[DEBUG]: " << configuration.clients_hostname << ":" << configuration.clients_port << std::endl;
	net::server clients_server(configuration.clients_hostname, configuration.clients_port, net::protocol::TCP, configuration.clients_number_of_connections, configuration.clients_server_secure_connection);
	if(configuration.server_secure_connection)
		clients_server.set_certificate_path(configuration.server_certificate, configuration.server_certificate_key);
	clients_server.listen();	

	while(true)
	{
		auto new_client = clients_server.accept_connection();
		std::cout << "[DEBUG]:" << new_client.get_host() << ":" << new_client.get_port() << std::endl;
		std::cout.flush();
		auto throw_away = std::async(handle_client, std::ref(new_client));
	}
	clients_server.close_connection();
}
