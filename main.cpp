#include "wsa_init.hpp"
#include <iostream>
#include <chrono>
#include <sstream>

int main() {
	try {
		WsaInit wsa;
		std::cout << "Winsock init\n";
	}
	catch (const std::exception& e){
		std::cerr << "Error: " << e.what() << "\n";
		return 1;
	}
	return 0;
}