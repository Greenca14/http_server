#include "wsa_init.hpp"
#include "socket.hpp"
#include <iostream>
#include <chrono>
#include <sstream>

int main() {
	try {
		WsaInit wsa;
		std::cout << "Winsock init\n";

		SOCKET raw = ::socket(AF_INET, SOCK_STREAM, 0);
		if (raw == INVALID_SOCKET) {
			std::cerr << "socket() failed: " << WSAGetLastError() << "\n";
			return 1;
		}
		std::cout << "2. Raw socket created: " << raw << "\n";

		Socket a(raw);
		std::cout << "3. a.valid() = " << (a.valid() ? "yes" : "no") << "\n";

		Socket b = std::move(a);
		std::cout << "4. After move: a.valid() = " << (a.valid() ? "yes" : "no") << ", b.valid() = " << (b.valid() ? "yes" : "no") << "\n";

		Socket c;
		std::cout << "5. c.valid() before = " << (c.valid() ? "yes" : "no") << "\n";
		c = std::move(b);
		std::cout << "6. After assign: b.valid() = " << (b.valid() ? "yes" : "no") << ", c.valid() = " << (c.valid() ? "yes" : "no") << "\n";
		
		SOCKET raw2 = ::socket(AF_INET, SOCK_STREAM, 0);
		Socket d(raw2);
		std::cout << "7. d.valid() = " << (d.valid() ? "yes" : "no") << "\n";

		c = std::move(d);
		std::cout << "8. After overwrite: c.valid() = " << (c.valid() ? "yes" : "no") << ", d.valid() = " << (d.valid() ? "yes" : "no") << "\n";
	
	}
	catch (const std::exception& e){
		std::cerr << "Error: " << e.what() << "\n";
		return 1;
	}
	return 0;
}