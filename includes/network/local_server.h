#pragma once

#include "network.h"
#include "accepted_client.h"

#include <sys/un.h>
#include <functional>
#include <signal.h>
#include <algorithm>

namespace net {
	class local_server : public network {
		uint32_t number_of_connections = 10;
		std::string certificate_path;
		std::string certificate_key_path;

		void setup_ssl();
		net::accepted_client accept_secure_tcp_connection();
		net::accepted_client accept_plaintext_tcp_connection();

		public:
			local_server();
			local_server(const std::string& socket_path, const uint32_t& number_of_connections = 10, const bool& secure = true);
		
			void set_certificate_path(const std::string& cert_path, const std::string& key_path = "");

			void listen();

			net::accepted_client accept_connection();
		
			void     set_max_number_of_connections(const uint32_t& number);
			uint32_t get_max_number_of_connections() const;
			
			void select(std::function<void(net::accepted_client&)> handle_client, int seconds = 0);

			void close_connection();
			void error_handle(const std::string& e) const override;
	};
}
