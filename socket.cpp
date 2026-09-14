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