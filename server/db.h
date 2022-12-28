#pragma once

#include <pqxx/pqxx>
#include "server_config.h"

pqxx::connection *db_connection;

class DB_CONN
{
	private:
		static pqxx::connection *db_connection;
	public:
		pqxx::connection instance() {
			if(db_connection == nullptr)
				db_connection = pqxx::connection{};
		}
};
