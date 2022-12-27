#include "accepted_client.h"

/*
net::accepted_client::accepted_client(const net::accepted_client& client)
{
	this->host = client.host;
	this->port = client.port;
	this->socket_type = client.socket_type;
	this->socketfd = client.socketfd;
	this->ssl_socket = client.ssl_socket;
	this->ssl_enabled = client.ssl_enabled;

	memcpy(&this->client_addr, &client.client_addr, sizeof(struct sockaddr_storage));
	this->client_addr_len = client.client_addr_len;	
	this->connection = client.connection;
}
*/
net::accepted_client::accepted_client(int& sock, SSL *secure, const net::protocol& prot, const struct sockaddr_storage& client_info, const socklen_t& length)
{
	char host_buffer[NI_MAXHOST], port_buffer[NI_MAXSERV];
	int result = getnameinfo((struct sockaddr *)& client_info, length, host_buffer, NI_MAXHOST,
								port_buffer, NI_MAXSERV, NI_NUMERICSERV | NI_NUMERICHOST);
	if(result != 0)
	{
		error_handle("accepted_client: getnameinfo cannot resolve address");
	}
	this->socketfd   = sock;
	this->ssl_socket = secure;
	this->host = std::string(host_buffer);
	this->port = atoi(port_buffer);
	this->socket_type = prot;
	
	if(secure == nullptr)
		this->ssl_enabled = false;
	else 
		this->ssl_enabled = true;

	this->connection  = true;
}

void net::accepted_client::set_host(const std::string& host)
{
	error_handle("set_host: cannot set a host in accepted_client");
}
void net::accepted_client::set_port(const uint16_t& port)
{
	error_handle("set_host: cannot set a port in accepted_client");
}

void net::accepted_client::set_socktype(const net::protocol& sockt)
{
	error_handle("set_host: cannot set a protocol in accepted_client");
}

void net::accepted_client::error_handle(const std::string& e) const
{
	net::network_error accepted_client_error("net::accepted_client::");
	accepted_client_error.add(e);
	throw accepted_client_error;
}

void send_message_plaintext_tcp(const int& socketfd, const std::string& s)
{
	uint64_t length = s.length();
	write(socketfd, (uint64_t *)&length, sizeof(uint64_t));
	write(socketfd, s.c_str(), s.length());
}

void send_message_secure_tcp(SSL* ssl_sock, const std::string& s)
{
	uint64_t length = s.length();	
	SSL_write(ssl_sock, (uint64_t *)&length, sizeof(uint64_t));
	SSL_write(ssl_sock, s.c_str(), s.length());
}

void send_message_plaintext_udp(const int& socketfd, const std::string& s, struct sockaddr_storage client_addr, socklen_t client_addr_len)
{
	uint64_t length = s.length();
	sendto(socketfd, &length, sizeof(uint64_t), 0, (struct sockaddr *)&client_addr, client_addr_len);
	sendto(socketfd, s.c_str(), length, 0, (struct sockaddr *)&client_addr, client_addr_len);
}

void net::accepted_client::send_message(const std::string& s) const
{
	if(this->socket_type == net::protocol::UDP)
	{
		send_message_plaintext_udp(this->socketfd, s, this->client_addr, this->client_addr_len);
	} else {
		if(this->ssl_enabled) {
			send_message_secure_tcp(this->ssl_socket, s);
		} else {
			send_message_plaintext_tcp(this->socketfd, s);
		}
	}
}


std::string& recv_message_plaintext_tcp(const int& socketfd)
{
	uint64_t length = 0;
	read(socketfd, (uint64_t *)&length, sizeof(uint64_t));
	char *buffer = new char[length+1];
	read(socketfd, buffer, length);
	std::string *s = new std::string(buffer, length);
	delete[] buffer;
	return *s;
}

std::string& recv_message_secure_tcp(SSL *ssl_socket)
{
	uint64_t length = 0;
	SSL_read(ssl_socket, &length, sizeof(uint64_t));
	char *buffer = new char[length+1];
	SSL_read(ssl_socket, buffer, length);
	std::string *s = new std::string(buffer, length);
	delete[] buffer;
	return *s;
}

std::string& recv_message_plaintext_udp(const int& socketfd, struct sockaddr_storage& client_addr, socklen_t& client_addr_len)
{
	uint64_t length = 0;
	recvfrom(socketfd, &length, sizeof(uint64_t), 0, (struct sockaddr *)&client_addr, &client_addr_len);
	char *buffer = new char[length+1];
	recvfrom(socketfd, buffer, length, 0, (struct sockaddr *)&client_addr, &client_addr_len);
	std::string *s = new std::string(buffer, length);
	delete[] buffer;
	return *s;
}

std::string net::accepted_client::receive_message()
{
	std::string s = "";
	if(this->socket_type == net::protocol::UDP)
	{
		struct sockaddr_storage client;
		socklen_t client_len;
		s = recv_message_plaintext_udp(this->socketfd, client, client_len);
	} else {
		if(this->ssl_enabled) {
			s = recv_message_secure_tcp(this->ssl_socket);
		} else {
			s = recv_message_plaintext_tcp(this->socketfd);
		}
	}
	if(s.length() == 0)
	{
		this->connection = false;
	}
	return s;
}

int net::accepted_client::get_socket() const
{
	return this->socketfd;
}

SSL *net::accepted_client::get_ssl_socket() const
{
	return this->ssl_socket;
}

bool net::accepted_client::is_connected() const 
{
	return this->connection;
}
