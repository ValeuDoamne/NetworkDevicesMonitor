#include "server.h"


net::server::server()
	:network("", 0, UNDEF), number_of_connections(10) { }


net::server::server(const std::string& host, const uint16_t& port, const net::protocol& socktype, const uint32_t& number_of_connections, const bool& secure_connection)
	:network(host, port, socktype, secure_connection), number_of_connections(number_of_connections) { }


bool net::server::try_to_bind(const struct addrinfo *rp)
{
	this->socketfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);

	if(this->socketfd == -1)
		return false;

	if(bind(this->socketfd, rp->ai_addr, rp->ai_addrlen) == -1){
		close(this->socketfd);
		return false;
	}

	return true;
}

void net::server::set_certificate_path(const std::string& certificate_path, const std::string& certificate_key_path)
{
	this->certificate_path = certificate_path;
	this->certificate_key_path = certificate_key_path;
}

void net::server::set_max_number_of_connections(const uint32_t& number)
{
	this->number_of_connections = number;
}

uint32_t net::server::get_max_number_of_connections() const
{
	return this->number_of_connections;
}

void initialcheck(net::server& c)
{

	if(c.get_port() == 0)
	{
		c.error_handle("listen: No port to listen provided");
	}

	if(c.get_socktype() == net::UNDEF)
	{
		c.error_handle("listen: No socket type provided");
	}
}

void net::server::setup_ssl()
{
	OpenSSL_add_all_algorithms();
	SSL_load_error_strings();
	const SSL_METHOD *method = TLS_server_method();
	this->ssl_ctx = SSL_CTX_new(method);
	if(this->ssl_ctx == nullptr)
	{
		error_handle("setup_ssl: Cannot initailize context");
	}
	if(SSL_CTX_use_certificate_file(this->ssl_ctx, this->certificate_path.c_str(), SSL_FILETYPE_PEM) <= 0)
	{
		error_handle("setup_ssl: Cannot load "+this->certificate_path+" certificate");
	}
	if(SSL_CTX_use_PrivateKey_file(this->ssl_ctx, this->certificate_key_path.c_str(), SSL_FILETYPE_PEM) <= 0)
	{
		error_handle("setup_ssl: Cannot load "+this->certificate_key_path+" key of certificate");
	}
	if(!SSL_CTX_check_private_key(this->ssl_ctx))
	{
		error_handle("setup_ssl: private key does not match the public key");
	}
}

void initialize_hints(struct addrinfo& hints, int ai_family, int socktype, int ai_flags = 0, int ai_protocol = 0)
{
	memset(&hints, 0, sizeof(hints));
	hints.ai_family   = ai_family;
	hints.ai_socktype = socktype;
	hints.ai_flags    = 0;
	hints.ai_protocol = 0;
}

void net::server::listen()
{
	signal(SIGPIPE, SIG_IGN);
	initialcheck(*this);
	
	if(this->ssl_enabled && this->socket_type == net::protocol::TCP)
	{
		this->setup_ssl();
	}

	struct addrinfo hints;
	struct addrinfo *result, *rp;
	initialize_hints(hints, AF_UNSPEC, this->socket_type);

	if(getaddrinfo(this->host.c_str(), std::to_string(this->port).c_str(), &hints, &result) != 0)
	{
		error_handle("connect: getaddrinfo cannot connect");
	}

	for(rp = result; rp != nullptr; rp = rp->ai_next)
	{
		if(this->try_to_bind(rp))
			break;
	}
	freeaddrinfo(result);

	if(rp == nullptr)
	{
		error_handle("listen: Could not connect");
	}
	if(this->socket_type == net::protocol::TCP)
	{
		const int enabled = 1;
		if (setsockopt(this->socketfd, SOL_SOCKET, SO_REUSEADDR, &enabled, sizeof(int)) < 0)
		{
			error_handle("setsockopt: SO_REUSEADDR failed");
		}
		this->listen_for_connections();
	}
}

void net::server::reset()
{
	this->close_connection();
}

void net::server::listen_for_connections()
{
	if(::listen(this->socketfd, this->number_of_connections) == -1)
	{
		error_handle("listen_for_connections: Cannot listen for connections");
	}
}

net::accepted_client net::server::accept_secure_tcp_connection()
{
	struct sockaddr_storage client_info;
	socklen_t client_info_len = sizeof(client_info);
	int client_sockfd = accept(this->socketfd, (struct sockaddr *)&client_info, &client_info_len);
	if(client_sockfd == -1)
	{
		error_handle("accept_connection: Cannot accept new connection");
	}
	SSL *ssl = SSL_new(this->ssl_ctx);
	SSL_set_fd(ssl, client_sockfd);
	
	if(SSL_accept(ssl) <= 0)
	{
		error_handle("accept_connection: Cannot accept new ssl hanshake");
	}

	net::accepted_client *new_client = new net::accepted_client{client_sockfd, ssl, this->socket_type, client_info, client_info_len};
	return *new_client;
}

net::accepted_client net::server::accept_plaintext_tcp_connection()
{
	struct sockaddr_storage client_info;
	socklen_t client_info_len = sizeof(client_info);
	int client_sockfd = accept(this->socketfd, (struct sockaddr *)&client_info, &client_info_len);
	if(client_sockfd == -1)
	{
		error_handle("accept_connection: Cannot accept new connection");
	}
	net::accepted_client *new_client = new net::accepted_client{client_sockfd, nullptr, this->socket_type, client_info, client_info_len};
	return *new_client;
}

net::accepted_client net::server::accept_plaintext_udp_connection()
{
	struct sockaddr_storage client_info;
	socklen_t client_info_len = sizeof(client_info);
	uint64_t length = 0;
	char buff[8];
	recvfrom(this->socketfd, &length, 8, 0, (struct sockaddr *)&client_info, &client_info_len);
	recvfrom(this->socketfd, buff, length, 0, (struct sockaddr *)&client_info, &client_info_len);
	int client_sockfd = this->socketfd;
	net::accepted_client *new_client = new net::accepted_client{client_sockfd, nullptr, this->socket_type, client_info, client_info_len};
	return *new_client;
}

net::accepted_client net::server::accept_connection()
{
	if(this->socket_type == net::protocol::TCP) {
		if(this->ssl_enabled) {
			return this->accept_secure_tcp_connection();
		} else {
			return this->accept_plaintext_tcp_connection();
		}
	} else if(this->socket_type == net::protocol::UDP) {
		return this->accept_plaintext_udp_connection();
	} 
	this->error_handle("accept_connection: No such protocol");
}

void net::server::select(std::function<void(net::accepted_client&)> handle_client, int seconds)
{
	if(this->socket_type != net::protocol::TCP)
	{
		error_handle("select: only implemented for multiplexing TCP connections");
	}

	fd_set readfds, actfds;
	int nfds = this->socketfd;
	struct timeval tv;

	std::vector<net::accepted_client> clients;
	
	FD_ZERO(&actfds);
	FD_SET(this->socketfd, &actfds);
	
	while(true) {
		tv.tv_sec = seconds;
		tv.tv_usec = 0;
		
		bcopy(&actfds, &readfds, sizeof(fd_set));
		
		int retval = ::select(nfds+1, &readfds, NULL, NULL, &tv);
		
		if(retval == -1)
		{
			this->error_handle(std::string("select: ")+strerror(errno));

		} else if(retval > 0) {
			int new_nfds = nfds;
			if(FD_ISSET(this->socketfd, &readfds) && clients.size() <= this->number_of_connections)
			{
				net::accepted_client client = this->accept_connection();
				
				if(clients.size() <= this->number_of_connections) {
					FD_SET(client.get_socket(), &actfds);
					clients.emplace_back(client);
					new_nfds = nfds < client.get_socket() ? client.get_socket() : nfds;
				} else {
					client.close_connection();
				}
			}
			nfds = new_nfds;
			for(auto& client : clients)
			{
				if(FD_ISSET(client.get_socket(), &readfds))
				{
					handle_client(client);
				}
				if(client.is_connected() == false)
				{
					FD_CLR(client.get_socket(), &actfds);
					client.close_connection();
					clients.erase(std::remove_if(clients.begin(), clients.end(), [](auto &c){return !c.is_connected();}), clients.end());
				}
			}
		}
	}
}


void net::server::close_connection() const
{
	close(this->socketfd);
}


void net::server::error_handle(const std::string& e) const
{
	net::network_error server_error("net::server::");
	server_error.add(e);
	throw server_error;
}
