#include "Threadpool.h"

ThreadPool::ThreadPool(ThreadPool::size_type threads) : stop(false) 
{
	workers.reserve(threads);

	for(ThreadPool::size_type i = 0; i < threads; ++i)
		workers.emplace_back(Worker(*this));
}

ThreadPool::~ThreadPool() {
	stop = true;

	condition.notify_all();

	for(ThreadPool::size_type i = 0; i < workers.size(); ++i)
		workers[i].join();
}

void Worker::operator()() 
{
	while(true) 
	{
		std::unique_lock<std::mutex> lock(pool.queue_mutex);

		while(!pool.stop && pool.tasks.empty())
			pool.condition.wait(lock);

		if(pool.stop && pool.tasks.empty())
			return;

		std::function<void()> task(pool.tasks.top().second);
		pool.tasks.pop();
		lock.unlock();
		task();
	}
}


