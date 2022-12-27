#pragma once

#include "network.h"

namespace net
{
	class accepted_client : public network {
		struct sockaddr_storage client_addr;
		socklen_t client_addr_len;
	
		bool connection;
	public:
		accepted_client();
		//accepted_client(const net::accepted_client& client);
		accepted_client(int& sock, SSL* secure, const net::protocol& prot, const struct sockaddr_storage& client_info, const socklen_t& length);
		
		void set_host(const std::string& host) override;
		void set_port(const uint16_t& port) override;
		void set_socktype(const net::protocol& sockt) override;
		
		void error_handle(const std::string& e) const override;
		
		void send_message(const std::string& s) const;
		std::string receive_message();

		int get_socket() const;
		SSL *get_ssl_socket() const;

		bool is_connected() const;
	};
}
