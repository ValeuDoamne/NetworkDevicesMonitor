#pragma once
#include <string>
struct Config {
	/* Server Agent Setup */
	std::string hostname;
	uint16_t    port;
	bool        server_secure_connection;
	uint32_t    number_of_connections;
	std::string server_certificate;
	std::string server_certificate_key;
	
	/* Server desktop clients */
	std::string clients_hostname;
	uint16_t    clients_port;
	bool        clients_server_secure_connection;
	uint32_t    clients_number_of_connections;
	std::string clients_server_certificate;
	std::string clients_server_certificate_key;

	/* Connection with database*/ 
	std::string db_host;
	uint16_t    db_port;
	std::string username;
	std::string password;
	std::string db_name;
	std::string db_users;
};

