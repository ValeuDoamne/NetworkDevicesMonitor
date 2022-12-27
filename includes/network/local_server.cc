
#include "local_server.h"

net::local_server::local_server()
	:network("/var/run/agent.pid", 0, TCP, true), number_of_connections(10) { }

net::local_server::local_server(const std::string& socket_path, const uint32_t& number_of_connections, const bool&secure)
	:network(socket_path, 0, TCP, secure), number_of_connections(number_of_connections) 
{
	if(access( socket_path.c_str(), F_OK ) != -1) {
		remove(socket_path.c_str());
	}
}

void net::local_server::set_max_number_of_connections(const uint32_t& number)
{
	this->number_of_connections = number;
}

uint32_t net::local_server::get_max_number_of_connections() const
{
	return this->number_of_connections;
}

void net::local_server::set_certificate_path(const std::string& cert_path, const std::string& key_path)
{
	this->certificate_path = cert_path;
	this->certificate_key_path = key_path;
}

void net::local_server::setup_ssl()
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
	if(certificate_key_path != ""){	
		if(SSL_CTX_use_PrivateKey_file(this->ssl_ctx, this->certificate_key_path.c_str(), SSL_FILETYPE_PEM) <= 0)
		{
			error_handle("setup_ssl: Cannot load "+this->certificate_key_path+" key of certificate");
		}
		
		if(!SSL_CTX_check_private_key(this->ssl_ctx))
		{
			error_handle("setup_ssl: private key does not match the public key");
		}
	}

}

char filename[108];

void signal_handler(int signum)
{
	remove(filename);
	signal(signum, SIG_DFL);
	kill(getpid(), signum);
}

void setup_signal_handlers(void (*signal_handler)(int))
{
	signal(SIGPIPE, SIG_IGN);
	signal(SIGINT , signal_handler);
	signal(SIGKILL, signal_handler);
	signal(SIGTERM, signal_handler);
	
}

void net::local_server::listen()
{
	strncpy(filename, this->get_host().c_str(), this->get_host().length());	

	setup_signal_handlers(signal_handler);

	if(this->ssl_enabled && this->socket_type == net::protocol::TCP)
	{
		this->setup_ssl();
	}


	this->socketfd = socket(AF_UNIX, this->socket_type, 0);
	if(this->socketfd == -1)
	{
		error_handle("listen: Cannot create socket");
	}
	struct sockaddr_un server_address;
	memset(&server_address, 0, sizeof(sockaddr_un));
	server_address.sun_family = AF_UNIX;
	strncpy(server_address.sun_path, this->get_host().c_str(), this->get_host().length());

	if(bind(this->socketfd, (sockaddr *)&server_address, sizeof(server_address)) == -1)
	{
		error_handle("listen: Cannot bind socket");
	}

	if(::listen(this->socketfd, this->number_of_connections) == -1)
	{
		error_handle("listen: Cannot make the socket listening");
	}

	const int enabled = 1;
	if (setsockopt(this->socketfd, SOL_SOCKET, SO_REUSEADDR, &enabled, sizeof(int)) < 0)
	{
		error_handle("setsockopt: SO_REUSEADDR failed");
	}

}

net::accepted_client net::local_server::accept_plaintext_tcp_connection()
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

net::accepted_client net::local_server::accept_secure_tcp_connection()
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

net::accepted_client net::local_server::accept_connection()
{
	if(this->ssl_enabled) {
		return this->accept_secure_tcp_connection();
	} else {
		return this->accept_plaintext_tcp_connection();
	}
}


void net::local_server::select(std::function<void(net::accepted_client&)> handle_client, int seconds)
{
	if(this->socket_type != net::protocol::TCP)
	{
		error_handle("select: only implemented for multiplexing TCP connections");
	}

	static fd_set readfds, actfds;
	
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
					printf("Socket: %d\n", client.get_socket());
					clients.push_back(client);
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


void net::local_server::close_connection()
{
	remove(this->get_host().c_str());
	shutdown(this->socketfd, SHUT_RDWR);
	close(this->socketfd);
}

void net::local_server::error_handle(const std::string& error) const
{
	net::network_error e("net::local_server::");
	e.add(error);
	throw e;
}
