#include "wsa_init.hpp"
#include "socket.hpp"
#include <iostream>
#include <chrono>
#include <sstream>

int main() {
	try {
		WsaInit wsa;
		
		Socket server(::socket(AF_INET, SOCK_STREAM, 0));
		if (!server.valid()) {
			std::cerr << "socket() failed: " << WSAGetLastError() << "\n";
			return 1;
		}
		std::cout << "Socket created\n";

		server.bind_to(8080);
		std::cout << "Set reuseaddr and bound to 8080\n";

		server.listen_on();
		std::cout << "Listening...\n";

		std::cout << "Press to exit (to netstat)\n";
		std::cin.get();
	}
	catch (const std::exception& e){
		std::cerr << "Error: " << e.what() << "\n";
		return 1;
	}
	return 0;
}