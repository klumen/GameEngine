#pragma once

#include "runtime/core/Memory.h"

#include <functional>
#include <future>
#include <thread>
#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>

namespace Lumen
{
	// TODO: priority, progress trace
	class ThreadPool
	{
	public:
		explicit ThreadPool(uint32_t threads = std::thread::hardware_concurrency())
		{
			if (threads == 0)
				threads = 1;
			m_Threads.reserve(threads);
			for (uint32_t i = 0; i < threads; ++i)
			{
				m_Threads.emplace_back([this]() {
					while (true)
					{
						std::function<void()> task;
						{
							std::unique_lock<std::mutex> lock(m_QueueMutex);
							m_Condition.wait(lock, [this]() { return !m_Running || !m_Tasks.empty(); });
							// equivalent to:
							/*while (!(!m_Running || !m_Tasks.empty()))
							{
								m_Condition.wait(lock);
							}*/
							if (!m_Running && m_Tasks.empty())
								return;
							task = std::move(m_Tasks.front());
							m_Tasks.pop();
						}
						task();
					}
					});
			}
		}

		~ThreadPool()
		{
			{
				std::scoped_lock<std::mutex> lock(m_QueueMutex);
				m_Running = false;
			}
			m_Condition.notify_all();
			for (auto& thread : m_Threads)
				thread.join();
		}
		
		template<typename F>
		std::future<std::invoke_result_t<F>> AddTask(F&& f)
		{
			auto task = MakeShared<std::packaged_task<std::invoke_result_t<F>()>>(std::forward<F>(f));

			auto res = task->get_future();  
			{
				std::scoped_lock<std::mutex> lock(m_QueueMutex);
				m_Tasks.emplace([task]() { (*task)(); });
			}
			m_Condition.notify_one();
			return res;
		}

	private:
		ThreadPool(const ThreadPool&) = delete;
		ThreadPool& operator=(const ThreadPool&) = delete;

		std::vector<std::thread> m_Threads;
		std::queue<std::function<void()>> m_Tasks;
		std::mutex m_QueueMutex;
		std::condition_variable m_Condition;
		bool m_Running = true;
	};
}