
#include "desktop_server.h"

#include "desktop_commands.h"

#include <future>
#include <vector>
#include <iostream>
#include <thread>
#include <json/json.h>
#include <pthread.h>

static pqxx::connection *db_connection;
static pqxx::connection *db_connection_users;

std::string handle_command(const Json::Value& root)
{
	static CommandManager manager;
	Json::Value result;
	Json::FastWriter w;;
	manager.add_command((Command *)(new LogIn(db_connection_users)));
	manager.add_command((Command *)(new LogOut));
	manager.add_command((Command *)(new GetData(db_connection)));

	auto received_command = root["command"].as<std::string>();
	for(const auto& command : manager.get_commands())
	{
		if(command->name() == received_command)
		{
			result = command->command(root);
			break;
		}
	}
	return w.write(result);
}

void *handle_client(void *client_ptr)
{
	net::accepted_client client = *(net::accepted_client *)client_ptr;
	while(client.is_connected())
	{
		auto json = client.receive_message();
		Json::Value root;
		try {
			std::istringstream stream(json);
			stream >> root;
		} catch (const std::exception& e) {
			std::cerr << "[ERROR]: Desktop clients server received bad json input" << std::endl;
		}

		auto send_back = handle_command(root);
		client.send_message(send_back);
	}
	client.close_connection();
	pthread_exit(0);
}


void setup_db_commands(const Config& configuration)
{
	db_connection = new pqxx::connection{"postgresql://"+configuration.username+":"+configuration.password+"@"+configuration.db_host+":"+std::to_string(configuration.db_port)+"/"+configuration.db_name};
	db_connection_users = new pqxx::connection{"postgresql://"+configuration.username+":"+configuration.password+"@"+configuration.db_host+":"+std::to_string(configuration.db_port)+"/"+configuration.db_users};

	db_connection->prepare("get_tables", "SELECT tablename FROM pg_catalog.pg_tables WHERE schemaname = 'public';");
	db_connection->prepare("get_columns", "SELECT string_agg(column_name, ', ' order by ordinal_position) as columns FROM information_schema.columns WHERE table_name = $1 GROUP BY table_name;");
	
	db_connection_users->prepare("login", "SELECT * FROM users WHERE username=$1 AND password=encode(digest($2, 'sha512'), 'hex');");

}

void desktop_server(const Config& configuration)
{
	setup_db_commands(configuration);

	net::server clients_server(configuration.clients_hostname, configuration.clients_port, net::protocol::TCP, configuration.clients_number_of_connections, configuration.clients_server_secure_connection);
	if(configuration.server_secure_connection)
		clients_server.set_certificate_path(configuration.server_certificate, configuration.server_certificate_key);
	clients_server.listen();	
	while(true)
	{
		auto new_client = new net::accepted_client{clients_server.accept_connection()};
		std::cout << "[INFO]:" << new_client->get_host() << ":" << new_client->get_port() << std::endl;
		std::cout.flush();
		//std::async(handle_client, new_client);
		//handle_client(new_client);
		pthread_t thread;
		pthread_create(&thread, NULL, handle_client, new_client);
		pthread_detach(thread);
	}
	clients_server.close_connection();
}
