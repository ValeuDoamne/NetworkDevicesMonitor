
#include "agent_server.h"

#include "../../network/server.h"

#include <iostream>
#include <future>
#include <json/json.h>
#include <pqxx/pqxx>
#include <pthread.h>

static pqxx::connection *db_connection;

void create_table(const Json::Value& agent_value)
{
	std::string query = "CREATE TABLE IF NOT EXISTS " + db_connection->esc(agent_value["table_name"].as<std::string>())+"(id SERIAL PRIMARY KEY, host INET, date DATE, ";
	auto columns = agent_value["columns"];
	for(int i = 0; i < columns.size(); i++)
	{
		auto column_name = columns[i]["column_name"].as<std::string>();
		auto column_type = columns[i]["type"].as<std::string>();
		
		if(i != columns.size()-1)
			query += db_connection->esc(column_name) + " " + column_type + ", ";
		else
			query += db_connection->esc(column_name) + " " + column_type + ");";
	}

	std::cerr << "[DEBUG]: Query:" << query << std::endl;
	std::cerr.flush();

	pqxx::work txn{*db_connection};
	txn.exec(query);
	txn.commit();
}


void insert_table(const Json::Value& agent_value, const std::string& host_str)
{
	
	pqxx::work txn{*db_connection};
	std::string query = "INSERT INTO "+txn.esc(agent_value["table_name"].as<std::string>())+"(host, date, ";
	std::string values = " VALUES ('" + host_str + "', CURRENT_DATE, " ;
	auto columns = agent_value["columns"];
	for(unsigned int i = 0; i < columns.size(); i++)
	{
		auto column_name = columns[i]["column_name"].as<std::string>();
		auto information = columns[i]["information"].as<std::string>();
		if(i != columns.size() - 1) {
			query += column_name + ",";
			values += "'" + txn.esc(information) + "',";
		} else {
			query += column_name + ")";
			values += "'"+txn.esc(information) + "');";
		}
	}
	query += values;
	std::cout << "Query: " << query << std::endl;
	
	txn.exec(query);
	txn.commit();
}

void process_json(const Json::Value& agent_value, const std::string& host_str)
{
	auto command = agent_value["cmd"].as<std::string>();
	if(command == "create_table") {
		create_table(agent_value);
	} else if(command == "insert_table") {
		insert_table(agent_value, host_str);
	}
}

void *handle_agent(void *agent_ptr)
{
	auto agent = *(net::accepted_client *)agent_ptr;
	while(agent.is_connected()) {
		auto json = agent.receive_message();
		std::cout << "Received from agent: " << json << std::endl;
		Json::Value agent_value;
		try {
			std::istringstream stream(json);
			stream >> agent_value;
		} catch (const std::exception& e) {
			std::cerr << "[Error]: " << e.what() << std::endl;
			continue; // not valid json
		}
		std::cout << agent_value.toStyledString() << std::endl;
		process_json(agent_value, agent.get_host());
	}
	agent.close_connection();
	pthread_exit(0);
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
		auto agent = new net::accepted_client{agent_server.accept_connection()};
		if(++thread_id < configuration.number_of_connections) {
			pthread_t thread;
			pthread_create(&thread, NULL, handle_agent, agent);
			pthread_detach(thread);
		} else {
			agent->close_connection();
		}

	}
	agent_server.close_connection();
	
	db_connection->close();
	delete db_connection;
}


