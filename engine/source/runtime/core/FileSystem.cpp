#include "runtime/core/FileSystem.h"

#include "runtime/function/global/GlobalContext.h"
#include "runtime/core/JobSystem.h"
#include "runtime/core/Log.h"
#include "runtime/core/Macro.h"

namespace Lumen
{
	void FileSystem::StartUp()
	{
	}

	void FileSystem::ShutDown()
	{
	}

	auto FileSystem::LoadFileAsync(const std::filesystem::path& path, std::vector<char>& out)
	{
		return LUMEN_JOB_SYSTEM->AddTask([&]() -> bool {
			std::ifstream file(path, std::ios::binary | std::ios::ate);
			if (!file.is_open())
			{
				LUMEN_RUNTIME_WARN("Cannot open the file {0}", path.string());
				return false;
			}
			auto size = file.tellg();
			file.seekg(0);
			out.resize(size);
			if (!file.read(out.data(), size))
			{
				LUMEN_RUNTIME_WARN("Cannot read the file {0}", path.string());
				return false;
			}
			return true;
			});
	}

	auto FileSystem::SaveFileAsync(const std::filesystem::path& path, std::vector<char>& data)
	{
		return LUMEN_JOB_SYSTEM->AddTask([=]() -> bool {
			std::ofstream file(path, std::ios::binary);
			if (!file.is_open())
			{
				LUMEN_RUNTIME_WARN("Cannot open the file {0}", path.string());
				return false;
			}
			if (!file.write(data.data(), data.size()))
			{
				LUMEN_RUNTIME_WARN("Cannot write the file {0}", path.string());
				return false;
			}
			return true;
			});
	}
}