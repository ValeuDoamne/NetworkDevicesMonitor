#pragma once

#include <exception>
#include <string>
#include <queue>

#include <openssl/ssl.h>
#include <openssl/err.h>

#include <cstdint>

#include <unistd.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "network_error.h"

#define DEFAULT_NO_OF_CONNECTIONS 1

namespace net
{
	enum protocol {TCP = SOCK_STREAM, UDP = SOCK_DGRAM, UNDEF = -1};

	class network {
	protected:
		std::string host;
		uint16_t port;
		protocol socket_type;
	
		bool ssl_enabled;
		SSL *ssl_socket = nullptr;
		SSL_CTX *ssl_ctx = nullptr;
		int socketfd;
	public:
		network(const std::string& host, const uint16_t& port, const net::protocol& sockt, const bool& ssl_enabled = true);
		network() = default;

		virtual void set_host(const std::string& host);
		virtual void set_port(const uint16_t& port);
		virtual void set_socktype(const net::protocol& sockt);

		std::string   get_host() const;
		uint16_t      get_port() const;
		net::protocol get_socktype() const;
		
		void close_connection() const;
	
		virtual void error_handle(const std::string& error) const;

		~network();
	};
}
