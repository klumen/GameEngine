#pragma once

#include "runtime/core/FileDialog.h"
#include "runtime/core/ThreadPool.h"

#include <filesystem>
#include <fstream>
#include <sstream>

namespace Lumen
{
	// singleton
	class FileSystem
	{
	public:
		FileSystem() {};
		~FileSystem() {};

		void StartUp();
		void ShutDown();

		std::filesystem::path SelectDirectory() { return FileDialog::SelectDirectory(); }
		std::filesystem::path SelectLoadFile(const char* filter) { return FileDialog::LoadFile(filter); }
		std::filesystem::path SelectSaveFile(const char* filter) { return FileDialog::SaveFile(filter); }

		auto LoadFileAsync(const std::filesystem::path& path, std::vector<char>& out);
		auto SaveFileAsync(const std::filesystem::path& path, std::vector<char>& data);

	private:
		FileSystem(const FileSystem&) = delete;
		FileSystem& operator=(const FileSystem&) = delete;

	};
}