#include "wsa_init.hpp"
#include "socket.hpp"
#include "thread_pool.hpp"
#include <iostream>
#include <chrono>
#include <sstream>
#include <cstring>
#include <windows.h>
#include <memory>

std::string make_response(int status, 
						const std::string& status_text,
						const std::string& content_type, 
						const std::string& body) {
	std::ostringstream oss;
	oss << "HTTP/1.1 " << status << " " << status_text << "\r\n"
		<< "Content-Type: " << content_type << "\r\n"
		<< "Content-Length: " << body.size() << "\r\n"
		<< "Connection: close\r\n"
		<< "\r\n"
		<< body;
	
	return oss.str();
}

std::string handle_request(const std::string& method, const std::string& path) {
	std::string clean_path = path;
	auto qpos = clean_path.find("?");
	if (qpos != std::string::npos) {
		clean_path = clean_path.substr(0, qpos);
	}

	if (clean_path == "/") {
		return make_response(200, "OK", "text/html", "<h1>Hello!</h1>");
	}
	if (clean_path == "/about") {
		return make_response(200, "OK", "text/html", "<h1>Ohaio!</h1>");
	}
	return make_response(404, "Not Found", "text/html", "<h1>404 Not Found</h1>");
}

int main() {
	try {
		WsaInit wsa;

		ThreadPool pool(6);
		std::cout << "Pool with 6 workers created\n";

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
					std::string request = client->recv_some();
					std::cout << "---Request\n" << request << "---End\n";

					std::istringstream iss(request);
					std::string method, path, version;
					iss >> method >> path >> version;

					std::cout << method	<< " " << path << " " << version << "\n";

					std::string response = handle_request(method, path);
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