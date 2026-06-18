#pragma once

#include <filesystem>
#include <thread>
#include <mutex>
#include <functional>

namespace Lumen
{
	enum class FileEventType : uint8_t
	{
		Added,
		Modified,
		Removed,
		RenameOld,
		RenameNew
	};

	struct FileEvent
	{
		FileEventType type;
		std::filesystem::path filePath;
		std::filesystem::path oldName;
		std::filesystem::path newName;
		bool isDirectory;
	};

	class FileWatch
	{
		using FileWatchCallbackFn = std::function<void(FileEvent&& event)>;

	public:
		explicit FileWatch(const std::filesystem::path& path, FileWatchCallbackFn callback);
		~FileWatch();

	private:
		void Watch();

	private:
		std::filesystem::path m_Path;
		FileWatchCallbackFn m_Callback;
		static constexpr std::size_t m_BufferSize = { 1024 * 256 };

		std::thread m_Thread;
		// std::mutex m_Mutex;
		// std::condition_variable m_Condition;

		std::atomic<bool> m_Destory{ false };

	};
}