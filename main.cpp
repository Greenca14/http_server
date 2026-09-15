#include "wsa_init.hpp"
#include "socket.hpp"
#include <iostream>
#include <chrono>
#include <sstream>
#include <cstring>
#include <windows.h>

int main() {
	try {
		WsaInit wsa;
		
		std::string body = "Hello!";
		std::ostringstream resp;
		resp << "HTTP/1.1 200 OK\r\n"
			<< "Content-Type: text/plain\r\n"
			<< "Content-Length: " << body.size() << "\r\n"
			<< "Connection: close\r\n"
			<< "\r\n"
			<< body;
		std::string response = resp.str();

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

		while (true) {
			std::cout << "Wait client\n";
			Socket client = server.accept_client();
			std::cout << "Client connected\n";

			int sent = ::send(client.get(), response.data(), response.size(), 0);
			if (sent == SOCKET_ERROR) {
				std::cerr << "send failed: " << WSAGetLastError() << "\n";
			}
			::shutdown(client.get(), SD_SEND);
			Sleep(100);
		}
	}
	catch (const std::exception& e){
		std::cerr << "Error: " << e.what() << "\n";
		return 1;
	}
	return 0;
}