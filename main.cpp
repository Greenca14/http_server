#include "wsa_init.hpp"
#include "socket.hpp"
#include "thread_pool.hpp"
#include <iostream>
#include <chrono>
#include <sstream>
#include <cstring>
#include <windows.h>
#include <memory>
#include <optional>
#include <fstream>

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

std::string mime_type(const std::string& path) {
	static const std::unordered_map<std::string, std::string> types = {
		{".html", "text/html"},
		{".htm", "text/html"},
		{".css", "text/css"},
		{".js", "application/javascript"},
		{".json", "application/json"},
		{".txt", "text/plain"},
		{".png",  "image/png"},
		{".jpg",  "image/jpeg"},
		{".jpeg", "image/jpeg"},
		{".gif",  "image/gif"},
		{".svg",  "image/svg+xml"},
		{".ico",  "image/x-icon"},
		{".pdf",  "application/pdf"},
	};

	auto dot = path.rfind(".");
	if (dot == std::string::npos) return "application/octet-stream";

	std::string ext = path.substr(dot);

	auto it = types.find(ext);
	if (it != types.end()) return it->second;

	return "application/octet-stream";
}

std::optional<std::string> read_file(const std::string& path) {
	std::ifstream file(path, std::ios::binary);
	if (!file) return std::nullopt;

	std::ostringstream oss;
	oss << file.rdbuf();
	return oss.str();
}

std::string handle_request(const std::string& method, const std::string& path) {
	if (method.empty()) {
		return make_response(400, "Bad Request", "text/html", "<h1>400 Bad Request</h1>");
	}

	if (method != "GET") {
		return make_response(405, "Method Not Allowed", "text/html", "<h1>405 Method Not Allowed</h1>");
	}

	if (path.find("..") != std::string::npos) {
		return make_response(400, "Bad Request", "text/html", "<h1>400 Bad Request</h1>");
	}
	
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
					std::string request = client->recv_request();
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