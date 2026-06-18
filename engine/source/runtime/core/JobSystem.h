#pragma once

#include "runtime/core/Memory.h"

#include <functional>
#include <future>
#include <thread>
#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <coroutine>

namespace Lumen
{
	enum class TaskPriority : uint8_t { Low, Normal, High };

	// TODO: base on Coroutine / Fiber
	// singleton
	class JobSystem
	{
	public:
		JobSystem() {};
		~JobSystem() {};

		void StartUp(uint32_t threads = std::thread::hardware_concurrency());
		void ShutDown();

		template<typename F>
		std::future<std::invoke_result_t<F>> AddTask(F&& f, TaskPriority priority = TaskPriority::Normal)
		{
			auto task = MakeShared<std::packaged_task<std::invoke_result_t<F>()>>(std::forward<F>(f));

			auto res = task->get_future();
			{
				std::scoped_lock<std::mutex> lock(m_Mutex);
				m_Tasks.emplace([task] { (*task)(); }, priority);
			}
			m_Condition.notify_one();
			return res;
		}

	private:
		JobSystem(const JobSystem&) = delete;
		JobSystem& operator=(const JobSystem&) = delete;

		struct TaskItem
		{
			std::function<void()> task;
			TaskPriority priority = TaskPriority::High;

			bool operator<(const TaskItem& other) const
			{
				return static_cast<uint8_t>(priority) < static_cast<uint8_t>(other.priority);
			}
		};

		std::vector<std::thread> m_Threads;
		std::priority_queue<TaskItem> m_Tasks;
		std::mutex m_Mutex;
		std::condition_variable m_Condition;
		bool m_Running = true;
	};
}