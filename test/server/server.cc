#include <iostream>
#include <string>

#include "network/network.h"
#include "network/server.h"
#include "network/accepted_client.h"

int main()
{
	std::string message;
	for(int i = 0; i < 999999; i++)
		message += "A";
	
	message += "B";
	net::server s{"127.0.0.1", 9999, net::TCP};
	s.set_certificate_path("./cert.pem", "./key.pem");
	s.listen();
	
	net::accepted_client c = s.accept_connection();
	c.send_message(message);
	return 0;
}
