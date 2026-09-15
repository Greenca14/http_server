#include "socket.hpp"

Socket::Socket() : sock_(INVALID_SOCKET) {}
Socket::Socket(SOCKET s) : sock_(s) {}

Socket::~Socket() {
	if (sock_ != INVALID_SOCKET) {
		::closesocket(sock_);
	}
}

Socket::Socket(Socket&& other) noexcept : sock_(other.sock_) {
	other.sock_ = INVALID_SOCKET;
}

Socket& Socket::operator=(Socket&& other) noexcept {
	if (this == &other) return *this;

	if (sock_ != INVALID_SOCKET) {
		::closesocket(sock_);
	}

	sock_ = other.sock_;
	other.sock_ = INVALID_SOCKET;
	
	return *this;
}

SOCKET Socket::get() const {
	return sock_;
}

bool Socket::valid() const {
	return sock_ != INVALID_SOCKET;
}

void Socket::bind_to(uint16_t port) {
	sockaddr_in addr{};
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = INADDR_ANY;
	
	int opt = 1;
	
	if (::setsockopt(sock_, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt)) == SOCKET_ERROR) {
		throw std::runtime_error("setsockopt failed: " + std::to_string(WSAGetLastError()));
	}
	if (::bind(sock_, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
		throw std::runtime_error("bind failed: " + std::to_string(WSAGetLastError()));
	}
}

void Socket::listen_on(int backlog) {
	if (!valid()) {
		throw std::runtime_error("listen called on invalid socket");
	}
	if (::listen(sock_, backlog) == SOCKET_ERROR) {
		throw std::runtime_error("listen failed: " + std::to_string(WSAGetLastError()));
	}
}

Socket Socket::accept_client() {
	if (sock_ == INVALID_SOCKET) {
		throw std::runtime_error("socket error: " + std::to_string(WSAGetLastError()));
	}
	SOCKET client = ::accept(sock_, nullptr, nullptr);
	if (client == INVALID_SOCKET) {
		throw std::runtime_error("client failed: " + std::to_string(WSAGetLastError()));
	}
	return Socket(client);
}

void Socket::send_all(const std::string& data) {
	size_t sent = 0;
	size_t total = data.size();

	while (sent < total) {
		int n = ::send(sock_, data.data() + sent, static_cast<int>(total - sent), 0);
		if (n == SOCKET_ERROR) {
			throw std::runtime_error("send failed: " + std::to_string(WSAGetLastError()));
		}
		if (n == 0) {
			throw std::runtime_error("send: connection closed");
		}

		sent += n;
	}
}