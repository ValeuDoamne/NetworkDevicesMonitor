#pragma once

#include <pqxx/pqxx>
#include "server_config.h"

pqxx::connection *db_connection;

void connect_to_database(const Config& configuration)
{
	db_connection = new pqxx::connection{"postgresql://"+configuration.username+":"+configuration.password+"@"+configuration.db_host+":"+std::to_string(configuration.db_port)+"/"+configuration.db_name};
		
}
