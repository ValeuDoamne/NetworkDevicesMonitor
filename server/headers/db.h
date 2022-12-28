#pragma once

#include <pqxx/pqxx>
#include "server_config.h"

pqxx::connection *db_connection;

class DB_CONN
{
	private:
		static pqxx::connection *db_connection;
	public:
		DB_CONN(const Config& configuration) {
			db_connection = new pqxx::connection{"postgresql://"+configuration.username+":"+configuration.password+"@"+configuration.db_host+":"+std::to_string(configuration.db_port)+"/"+configuration.db_name};
		}
		static pqxx::connection instance() {
			if(db_connection == nullptr)	
				db_connection = new pqxx::connection{"postgresql://"+configuration.username+":"+configuration.password+"@"+configuration.db_host+":"+std::to_string(configuration.db_port)+"/"+configuration.db_name};
			return *db_connection;
		}
};
