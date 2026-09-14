#pragma once

#include <WinSock2.h>
#include <stdexcept>
#include <string>

class WsaInit {
public:
	WsaInit() {
		int result = WSAStartup(MAKEWORD(2, 2), &wsa_data_);
		if (result != 0) { throw std::runtime_error("WSAStartup failed: " + std::to_string(result)); }
	}

	~WsaInit() {
		WSACleanup();
	}

	WsaInit(const WsaInit&) = delete;
	WsaInit& operator=(const WsaInit&) = delete;
private:
	WSADATA wsa_data_;

};