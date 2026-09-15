#include "wsa_init.hpp"
#include "socket.hpp"
#include "thread_pool.hpp"
#include <iostream>
#include <chrono>
#include <sstream>
#include <cstring>
#include <windows.h>
#include <memory>

static const std::string response = 
	"HTTP/1.1 200 OK\r\n"
	"Content-Type: text/plain\r\n"
	"Content-Length: 13\r\n"
	"Connection: close\r\n"
	"\r\n"
	"Hello!";

int main() {
	try {
		WsaInit wsa;

		ThreadPool pool(6);
		std::cout << "Pool with 4 workers created\n";

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

			pool.enqueue([client = std::make_shared<Socket>(std::move(client))]() mutable {
				try {
					client->send_all(response);
				}
				catch (const std::exception& e) {
					std::cerr << "send failed: " << e.what() << "\n";
				}
				::shutdown(client->get(), SD_SEND);
				Sleep(100);
			});
		}
	}
	catch (const std::exception& e){
		std::cerr << "Error: " << e.what() << "\n";
		return 1;
	}
	return 0;
}