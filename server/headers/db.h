#pragma once

#include <pqxx/pqxx>
#include "server_config_struct.h"

class DB_CONN
{
	private:
		static pqxx::connection *db_connection;
		static pqxx::connection *db_users;
	public:
		static void instance(const Config& configuration) {
			db_connection = new pqxx::connection{"postgresql://"+configuration.username+":"+configuration.password+"@"+configuration.db_host+":"+std::to_string(configuration.db_port)+"/"+configuration.db_name};
		}
		static void instance_users(const Config& configuration) {
			db_connection = new pqxx::connection{"postgresql://"+configuration.username+":"+configuration.password+"@"+configuration.db_host+":"+std::to_string(configuration.db_port)+"/"+configuration.db_users};
		}
		static pqxx::connection *get_instance_users() {
			return db_users;
		}
		static pqxx::connection *get_instance() {
			return db_connection;
		}
		static void dealocate()
		{
			delete db_connection;	
		}
};

