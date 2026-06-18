#include "runtime/core/FileWatch.h"

#include "runtime/function/global/GlobalContext.h"
#include "runtime/core/Log.h"
#include "runtime/core/Macro.h"

#ifdef LUMEN_PLATFORM_WINDOWS

#include <Windows.h>

namespace Lumen
{
	FileWatch::FileWatch(const std::filesystem::path& path, FileWatchCallbackFn callback)
		:  m_Path(path), m_Callback(callback)
	{
		m_Thread = std::thread([this]() { Watch(); });
	}
	
	FileWatch::~FileWatch()
	{
		m_Destory = true;
		m_Thread.join();
	}

	void FileWatch::Watch()
	{
		if (!std::filesystem::exists(m_Path))
		{
			LUMEN_RUNTIME_ERROR("Path does not exist: {0}", m_Path.string());
			return;
		}

		std::filesystem::path watchPath = 
			std::filesystem::is_directory(m_Path) ? m_Path : m_Path.parent_path();
		HANDLE directory = CreateFileW(
			watchPath.c_str(),
			FILE_LIST_DIRECTORY,
			FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
			nullptr,
			OPEN_EXISTING,
			FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED,
			nullptr);

		if (directory == INVALID_HANDLE_VALUE)
		{
			LUMEN_RUNTIME_ERROR("Unable to accquire directory handle: {0}", GetLastError());
			return;
		}

		std::vector<BYTE> buffer(m_BufferSize);
		OVERLAPPED overlapped{ 0 };
		DWORD bytesReturned = 0;
		ZeroMemory(&overlapped, sizeof(overlapped));
		
		overlapped.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
		if (!overlapped.hEvent)
		{
			LUMEN_RUNTIME_ERROR("Create file watch event failed!");
			CloseHandle(directory);
			return;
		}

		std::unordered_map<std::filesystem::path, std::filesystem::path> renameMap;

		while (!m_Destory)
		{
			DWORD status = ReadDirectoryChangesW(
				directory,
				buffer.data(),
				static_cast<DWORD>(buffer.size()),
				TRUE,
				/*FILE_NOTIFY_CHANGE_SECURITY, LAST_ACCESS, LAST_WRITE, SIZE, ATTRIBUTES*/
				FILE_NOTIFY_CHANGE_CREATION | FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME,
				&bytesReturned,
				&overlapped,
				nullptr);
			
			if (!status)
			{
				LUMEN_RUNTIME_ERROR(GetLastError());
				break;
			}

			DWORD waitOperation = WaitForSingleObject(overlapped.hEvent, INFINITE);
			if (waitOperation != WAIT_OBJECT_0)
			{
				LUMEN_RUNTIME_ERROR("WaitForSingleObject failed: {0}", GetLastError());
				break;
			}

			if (!GetOverlappedResult(directory, &overlapped, &bytesReturned, FALSE))
			{
				LUMEN_RUNTIME_ERROR("GetOverlappedResult failed: {0}", GetLastError());
				break;
			}

			std::filesystem::path oldName;
			BYTE* buf = buffer.data();

			for (;;)
			{
				FILE_NOTIFY_INFORMATION* fileInfo = reinterpret_cast<FILE_NOTIFY_INFORMATION*>(buf);
				std::wstring fileName{ fileInfo->FileName, fileInfo->FileNameLength / sizeof(fileInfo->FileName[0]) };
				std::filesystem::path filepath = watchPath / fileName;

				if (!std::filesystem::is_directory(m_Path))
					if (filepath != m_Path) continue;

				FileEvent e;
				e.filePath = filepath;
				e.newName = fileName;
				e.oldName = fileName;
				e.isDirectory = std::filesystem::is_directory(filepath);

				switch (fileInfo->Action)
				{
				case FILE_ACTION_ADDED:
				{
					e.type = FileEventType::Added;
					break;
				}
				case FILE_ACTION_REMOVED:
				{
					e.type = FileEventType::Removed;
					break;
				}
				case FILE_ACTION_MODIFIED:
				{
					e.type = FileEventType::Modified;
					break;
				}
				case FILE_ACTION_RENAMED_OLD_NAME:
				{
					e.type = FileEventType::RenameOld;
					oldName = fileName;
					break;
				}
				case FILE_ACTION_RENAMED_NEW_NAME:
				{
					e.type = FileEventType::RenameNew;
					e.oldName = oldName;
					break;
				}
				}

				// TODO: maybe add to thread pool?
				m_Callback(std::move(e));

				if (!fileInfo->NextEntryOffset)
					break;

				buf += fileInfo->NextEntryOffset;
			}
		}

		CloseHandle(directory);
		CloseHandle(overlapped.hEvent);

		return;
	}
}

#endif // LUMEN_PLATFORM_WINDOWS