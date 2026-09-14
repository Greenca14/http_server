#include "thread_pool.hpp"
#include <iostream>
#include <chrono>
#include <sstream>

int main() {
	ThreadPool pool(4);
	
	for (int i = 0; i < 10; ++i) {
		pool.enqueue([i] {
			std::ostringstream oss;
			oss << "Task " << i << " on thread " << std::this_thread::get_id();
			//std::cout << "Task " << i << " on thread " << std::this_thread::get_id() << std::endl;
			std::string res = oss.str();
			std::cout << res << "\n";
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
			});
	}

	std::this_thread::sleep_for(std::chrono::seconds(2));
	std::cout << "Main done" << std::endl;
}