#include "network.h"
#include <regex>

net::network::network(const std::string& host, const uint16_t& port, const net::protocol& prot, const bool& secure_connection)
	:host(host), port(port), socket_type(prot), ssl_enabled(secure_connection) {}

void net::network::set_host(const std::string& newhost)
{
	this->host = newhost;
}

void net::network::set_port(const uint16_t& newport)
{
	this->port = newport;
}

void net::network::set_socktype(const net::protocol& type)
{
	this->socket_type = type;
}

std::string net::network::get_host() const
{
	return this->host;
}

uint16_t net::network::get_port() const
{
	return this->port;
}


net::protocol net::network::get_socktype() const
{
	return this->socket_type;
}

void net::network::close_connection() const
{
	shutdown(this->socketfd, SHUT_RDWR);
	close(this->socketfd);
}

void net::network::error_handle(const std::string& message) const
{
	net::network_error error("net::network::");
	error.add(message);
	throw error;
}

net::network::~network()
{
	if(this->ssl_socket != nullptr) {
		SSL_free(this->ssl_socket);
		this->ssl_socket = nullptr;
	}
	if(this->ssl_ctx != nullptr) {
		SSL_CTX_free(this->ssl_ctx);
		this->ssl_ctx = nullptr;
	}
}
