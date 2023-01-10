#include <iostream>
#include "network/client.h"

int main()
{
	net::client c{"127.0.0.1", 9999, net::TCP, true};	
	c.connect();

	auto msg = c.receive_message();
	std::cout << "Received msg: " << msg << std::endl;
	std::cout << "Length: " << msg.length() << std::endl;
}
