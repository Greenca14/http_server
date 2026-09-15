#pragma once

#include <WinSock2.h>
#include <cstdint>
#include <stdexcept>
#include <string>

class Socket {
public:
	Socket();
	explicit Socket(SOCKET s);
	~Socket();

	Socket(const Socket&) = delete;
	Socket& operator=(const Socket&) = delete;

	Socket(Socket&& other) noexcept;
	Socket& operator=(Socket&& other) noexcept;

	SOCKET get() const;
	bool valid() const;

	void bind_to(uint16_t port);
	void listen_on(int backlog = SOMAXCONN);

private:
	SOCKET sock_;
};