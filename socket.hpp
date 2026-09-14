#pragma once

#include <WinSock2.h>

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

private:
	SOCKET sock_;
};