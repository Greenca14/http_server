#pragma once

#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>

class ThreadPool {
public:
	ThreadPool(size_t n);
	~ThreadPool();
	void enqueue(std::function<void()> task);
private:
	std::vector<std::thread> workers_;
	std::queue<std::function<void()>> tasks_;
	std::mutex mtx_;
	std::condition_variable cv_;
	bool stop_ = false;
	void worker_loop();
};

