#pragma once

#include "network.h"
#include "accepted_client.h"

#include <algorithm>
#include <functional>
#include <cerrno>

#include <sys/select.h>
#include <sys/signal.h>

namespace net
{
	class server : public network {
		uint32_t number_of_connections;
		std::string certificate_path;
		std::string certificate_key_path;

		bool try_to_bind(const struct addrinfo *rp);
		
		void setup_ssl();
		net::accepted_client accept_secure_tcp_connection(); 
		net::accepted_client accept_plaintext_tcp_connection();
		net::accepted_client accept_plaintext_udp_connection();
		void listen_for_connections();
	public:
		
		server();
		server(const std::string& host, const uint16_t& port, const protocol& sockt, const uint32_t& number_of_connections = 10, const bool& secure_connection = true);

		void     set_certificate_path(const std::string& certificate_path, const std::string& certificate_key_path = "");

		void     set_max_number_of_connections(const uint32_t& number);
		uint32_t get_max_number_of_connections() const;

		void listen();
		net::accepted_client accept_connection();
		void close_connection() const;
		void select(std::function<void (net::accepted_client &)> handle_client, int seconds = 0);
		void reset();

		void error_handle(const std::string& e) const override;
	};
}
