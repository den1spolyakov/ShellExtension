#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <vector>
#include <queue>
#include <memory>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <future>
#include <algorithm>
#include <chrono>
#include <utility>
#include <functional>

class ThreadPool;

class Worker
{
private:
	ThreadPool & pool;
public:
	Worker(ThreadPool & p) : pool(p) {}
	void operator()();
};

class ThreadPool
{
private:
	friend class Worker;
	std::vector<std::thread> workers;
	typedef std::pair<int, std::function<void()>> priority_task;

	struct task_comp 
	{
		bool operator()(const priority_task & lhs, 
			const priority_task & rhs) const 
		{
			return lhs.first > rhs.first;
		}
	};

	std::priority_queue<priority_task, std::vector<priority_task>, task_comp> tasks;
	std::mutex queue_mutex;
	std::condition_variable condition;
	bool stop;
public:
	typedef std::vector<std::thread>::size_type size_type;

	ThreadPool(size_type);
	~ThreadPool();
	template<class F>
	auto enqueue(F&& f, int priority = 0) -> std::future<decltype(std::forward<F>(f)())>;

};

template<class F>
auto ThreadPool::enqueue(F&& f, int priority) -> std::future<decltype(std::forward<F>(f)())> {
	typedef decltype(std::forward<F>(f)()) R;

	if(stop)
		throw std::runtime_error("enqueue on stopped threadpool");

	auto task = std::make_shared<std::packaged_task<R()>>(std::forward<F>(f));
	std::future<R> res = task->get_future();

	{
		std::unique_lock<std::mutex> lock(queue_mutex);
		tasks.emplace(priority, [task]{ (*task)(); });
	}

	condition.notify_one();

	return res;
}


#endif