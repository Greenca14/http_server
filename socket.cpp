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