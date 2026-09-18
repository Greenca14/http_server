#include "thread_pool.hpp"

ThreadPool::ThreadPool(size_t n) {
	for (size_t i = 0; i < n; ++i) {
		workers_.emplace_back([this] {worker_loop(); });
	}
}

ThreadPool::~ThreadPool() {
	{
		std::lock_guard<std::mutex> lock(mtx_);
		stop_ = true;
	}
	cv_.notify_all();
	for (auto& t : workers_) {
		if (t.joinable()) t.join();
	}
}

void ThreadPool::worker_loop() {
	while (true) {
		std::unique_lock<std::mutex> lock(mtx_);
		cv_.wait(lock, [this] { return stop_ || !tasks_.empty(); });
		if (stop_ && tasks_.empty()) {
			return;
		}
		std::function<void()> task = std::move(tasks_.front());
		tasks_.pop();
		lock.unlock();
		task();
	}
}

void ThreadPool::enqueue(std::function<void()> task) {
	{
		std::lock_guard<std::mutex> lock(mtx_);
		if (stop_) {
			return;
		}
		else {
			tasks_.push(std::move(task));
		}
	}
	cv_.notify_one();
}