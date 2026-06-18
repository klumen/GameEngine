#include "runtime/core/JobSystem.h"

namespace Lumen
{
	void JobSystem::StartUp(uint32_t threads/* = std::thread::hardware_concurrency()*/)
	{
		if (threads == 0) threads = 1;
		m_Threads.reserve(threads);
		for (uint32_t i = 0; i < threads; ++i)
		{
			m_Threads.emplace_back([this] {
				while (true)
				{
					TaskItem taskItem;
					{
						std::unique_lock<std::mutex> lock(m_Mutex);
						m_Condition.wait(lock, [this]() { return !m_Running || !m_Tasks.empty(); });
						// equivalent to:
						/*while (!(!m_Running || !m_Tasks.empty()))
						{
							m_Condition.wait(lock);
						}*/
						if (!m_Running && m_Tasks.empty())
							return;
						taskItem = m_Tasks.top();
						m_Tasks.pop();
					}
					taskItem.task();
				}
				});
		}
	}

	void JobSystem::ShutDown()
	{
		{
			std::scoped_lock lock(m_Mutex);
			m_Running = false;
		}

		m_Condition.notify_all();
		for (auto& thread : m_Threads)
			thread.join();
	}
}