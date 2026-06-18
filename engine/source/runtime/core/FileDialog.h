#pragma once

#include <filesystem>

namespace Lumen
{
	class FileDialog
	{
	public:
		FileDialog() = delete;

		static std::filesystem::path LoadFile(const char* filter);
		static std::filesystem::path SaveFile(const char* filter);
		static std::filesystem::path SelectDirectory();

	};
}