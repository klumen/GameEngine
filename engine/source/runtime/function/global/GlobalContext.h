#pragma once

#include "runtime/core/Memory.h"

#include <filesystem>

namespace Lumen
{
	class JobSystem;
	class Log;
	class ConfigManager;
	class FileSystem;
	class AssetManager;
	class Window;
	class RenderSystem;

	// TODO: may not singleton - for runtime and editor
	class GlobalContext
	{
	public:
		static inline GlobalContext& Instance()
		{
			static GlobalContext singleton;
			return singleton;
		}

		void StartUp(const std::filesystem::path& configFilePath);
		void ShutDown();

		inline Shared<JobSystem> GetJobSystem() const { return m_JobSystem; }
		inline Shared<FileSystem> GetFileSystem() const { return m_FileSystem; }
		inline Shared<Log> GetLog() const { return m_Log; }
		inline Shared<ConfigManager> GetConfigManager() const { return m_ConfigManager; }
		inline Shared<AssetManager> GetAssetManager() const { return m_AssetManager; }
		inline Shared<Window> GetWindow() const { return m_Window; }
		inline Shared<RenderSystem> GetRenderSystem() const { return m_RenderSystem; }

	private:
		GlobalContext();
		~GlobalContext();
		GlobalContext(const GlobalContext&) = delete;
		GlobalContext& operator=(const GlobalContext&) = delete;

		Shared<JobSystem> m_JobSystem;
		Shared<FileSystem> m_FileSystem;
		Shared<Log> m_Log;
		Shared<ConfigManager> m_ConfigManager;
		Shared<AssetManager> m_AssetManager;
		Shared<Window> m_Window;
		Shared<RenderSystem> m_RenderSystem;
	};
}