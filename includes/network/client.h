#pragma once
#include "network.h"

namespace net 
{
	class client : public network {
		bool local_connection;
		
		bool try_to_connect(const struct addrinfo *rp);

		void setup_ssl();
		
		void connect_remote();
		void connect_local();

		public:
		
		client(const std::string& socket_path, const bool& secure_connection = false)
			:network(socket_path, 0, TCP, secure_connection), local_connection(true) { }
		
		client(const std::string& host, const uint16_t& port, const protocol& sockt = net::protocol::TCP, const bool& secure_connection = true)
			:network(host, port, sockt, secure_connection), local_connection(false) { }

		std::string receive_message();
		void 	    send_message(const std::string& s);

		void connect();
		void reset();

		void error_handle(const std::string& e) const override;
	};
}
