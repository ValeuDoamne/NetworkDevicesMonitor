#include "client.h"
#include <cstring>
#include <sys/un.h>

bool net::client::try_to_connect(const struct addrinfo *rp)
{
	this->socketfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
	
	if(this->socketfd == -1)
		return false;
	
	if(::connect(this->socketfd, rp->ai_addr, rp->ai_addrlen) == -1){
		close(this->socketfd);
		return false;
	}

	return true;
}


void initialize_client_hints(struct addrinfo& hints, int ai_family, int socktype, int ai_flags = 0, int ai_protocol = 0)
{
	memset(&hints, 0, sizeof(hints));
	hints.ai_family   = ai_family;
	hints.ai_socktype = socktype;
	hints.ai_flags    = 0;
	hints.ai_protocol = 0;
}

void net::client::connect()
{
	if(this->local_connection)
	{
		this->connect_local();
	} else {
		this->connect_remote();
	}
	if(this->ssl_enabled && this->socket_type == net::protocol::TCP) {
		this->setup_ssl();
	}
}

void net::client::connect_local()
{
	this->socketfd = ::socket(AF_UNIX, this->socket_type, 0);
	
	if(this->socketfd == -1)
	{
		error_handle("connect: Cannot bind socket");
	}
	struct sockaddr_un local_server;
	bzero(&local_server, sizeof(sockaddr_un));
	local_server.sun_family = AF_UNIX;
	strncpy(local_server.sun_path, this->host.c_str(), this->host.length());
	socklen_t local_server_len = sizeof(local_server);

	if(::connect(this->socketfd, (sockaddr *)&local_server, local_server_len) == -1)
	{
		error_handle("connect: Cannot connect: "+std::string(local_server.sun_path));
	}
}

void net::client::connect_remote()
{

	struct addrinfo hints;
	struct addrinfo *result, *rp;
	initialize_client_hints(hints, AF_UNSPEC, this->socket_type);
	
	if(getaddrinfo(this->host.c_str(), std::to_string(this->port).c_str(), &hints, &result) != 0)
	{
		error_handle("connect: getaddrinfo cannot connect");
	}

	for(rp = result; rp != nullptr; rp = rp->ai_next)
	{
		if(this->try_to_connect(rp))
			break;
	}
	freeaddrinfo(result);

	if(rp == nullptr) {
		error_handle("connect_to_host: Could not connect");
	}
	if(this->socket_type == net::protocol::UDP)
	{
		this->send_message("start");
		return;
	}
}

void net::client::send_message(const std::string& s)
{
	uint64_t length = s.length();
	if(this->ssl_enabled) {
		SSL_write(this->ssl_socket, (uint64_t *)&length, sizeof(uint64_t));
		SSL_write(this->ssl_socket, s.c_str(), s.length());
	} else {
		write(this->socketfd, (uint64_t *)&length, sizeof(uint64_t));
		write(this->socketfd, s.c_str(), s.length());
	}
}

std::string net::client::receive_message()
{
	std::string *s;
	char *buffer;

	uint64_t length = 0;
	if(this->ssl_enabled){
		SSL_read(this->ssl_socket, &length, sizeof(uint64_t));
		buffer = new char[length+1];
		SSL_read(this->ssl_socket, buffer, length);
	} else {
		read(this->socketfd, &length, sizeof(uint64_t));
		buffer = new char[length+1];
		read(this->socketfd, buffer, length);
	}
	
	s = new std::string(buffer, length);

	delete[] buffer;
	return *s;
}


void net::client::setup_ssl()
{
	
	SSL_library_init();
	SSLeay_add_ssl_algorithms();
	SSL_load_error_strings();
	const SSL_METHOD *method = TLS_client_method();
	this->ssl_ctx = SSL_CTX_new(method);
	if(this->ssl_ctx == nullptr)
	{
		error_handle("create_connection: Cannot create secure context");
	}
	this->ssl_socket = SSL_new(this->ssl_ctx);
	SSL_set_fd(this->ssl_socket, this->socketfd);
	if(SSL_connect(this->ssl_socket) <= 0)
	{
		error_handle("create_connection: error in SSL handshake");
	}
}

void net::client::error_handle(const std::string& e) const
{
	net::network_error client_error("net::client::");
	client_error.add(e);
	throw client_error;
}

