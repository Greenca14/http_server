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
#include <filesystem>
#include <unordered_map>
#include <ctime>
#include <atomic>
#include <csignal>
 
namespace fs = std::filesystem;

static fs::path g_root = "public";
std::mutex g_log_mutex;
std::atomic<bool> g_shutdown{false};

void handle_signal(int) {
	g_shutdown = true;
}

struct Response {
	int status;
	std::string content;
};

std::string build_response(int status, 
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

Response make_response(int status,
						const std::string& status_text,
						const std::string& content_type,
						const std::string& body) {
	return Response{
		status,
		build_response(status, status_text, content_type, body)
	};
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

std::optional<fs::path> safe_path(const fs::path& root, const std::string& url_path) {
	std::string relative = url_path;
	if (!relative.empty() && (relative[0] == '/' || relative[0] == '\\')) {
		relative = relative.substr(1);
	}

	fs::path full = root / relative;

	fs::path canon_full, canon_root;
	try {
		canon_full = fs::weakly_canonical(full);
		canon_root = fs::weakly_canonical(root);
	}
	catch (const fs::filesystem_error&) {
		return std::nullopt;
	}

	if (canon_full == canon_root) {
		return std::nullopt;
	}

	fs::path rel = canon_full.lexically_relative(canon_root);
	auto first = rel.begin();
	if (first != rel.end() && *first == "..") {
		return std::nullopt;
	}

	return canon_full;
}	

Response handle_request(const std::string& method, const std::string& path, const fs::path& root) {
	if (method.empty()) {
		return make_response(400, "Bad Request", "text/html", "<h1>400 Bad Request</h1>");
	}

	if (method != "GET") {
		return make_response(405, "Method Not Allowed", "text/html", "<h1>405 Method Not Allowed</h1>");
	}
	
	std::string clean_path = path;
	auto qpos = clean_path.find("?");
	if (qpos != std::string::npos) {
		clean_path = clean_path.substr(0, qpos);
	}

	if (clean_path == "/") {
		clean_path = "/index.html";
	}

	auto safe = safe_path(root, clean_path);
	if (!safe) {
		return make_response(400, "Bad Request", "text/html", "<h1>400 Bad Request</h1>");
	}

	if (!fs::exists(*safe) || !fs::is_regular_file(*safe)) {
		return make_response(404, "Not Found", "text/html", "<h1>404 Not Found</h1>");
	}

	auto content = read_file(safe->string());
	if (!content) {
		return make_response(500, "Internal Server Error", "text/html", "<h1>500 Internal Server Error</h1>");
	}

	std::string mime = mime_type(safe->string());

	return make_response(200, "OK", mime, *content);
}

std::string current_time_string() {
	std::time_t now = std::time(nullptr);
	std::tm tm_buf;
	if (localtime_s(&tm_buf, &now) != 0) {
		return "???";
	}
	char buf[32];
	std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &tm_buf);
	return std::string(buf);
}

void log_request(const std::string& method, const std::string& path, int status, size_t size) {
	std::lock_guard<std::mutex> lock(g_log_mutex);
	std::cout << "[" << current_time_string() << "]" << method << " " << path << " -> " << status << " (" << size << " bytes)\n";
}

int main(int argc, char** argv) {
	if (argc > 1) g_root = argv[1];
	std::signal(SIGINT, handle_signal);
	try {
		WsaInit wsa;
		ThreadPool pool(6);
		Socket server(::socket(AF_INET, SOCK_STREAM, 0));

		if (!server.valid()) {
			std::cerr << "socket() failed: " << WSAGetLastError() << "\n";
			return 1;
		}

		server.bind_to(8080);
		server.listen_on();
		std::cout << "Listening on http://localhost:8080\n";

		while (!g_shutdown) {
			if (!server.wait_readable(200)) {
				continue;
			}

			Socket client = server.accept_client();

			pool.enqueue([client = std::make_shared<Socket>(std::move(client))]() mutable {
				try {
					std::string request = client->recv_request();

					std::istringstream iss(request);
					std::string method, path, version;
					iss >> method >> path >> version;


					Response response = handle_request(method, path, g_root);
					client->send_all(response.content);
					log_request(method, path, response.status, response.content.size());
				}
				catch (const std::exception& e) {
					std::cerr << "send failed: " << e.what() << "\n";
				}
				::shutdown(client->get(), SD_SEND);
				Sleep(100);
			});
		}
		std::cout << "\nShutdown signal received. Closing...\n";
	}
	catch (const std::exception& e){
		std::cerr << "Error: " << e.what() << "\n";
		return 1;
	}
	return 0;
}