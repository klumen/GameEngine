#pragma once

#include <SimpleIni.h>

#include <filesystem>

namespace Lumen
{
	// TODO: config options menus in-game (console variables)
	// per-user options in windows:  C:\Users\AppData or HKEY_CURRENT_USER(?)
	// TODO: plugins.cfg resources.cfg video.cfg
	enum class GraphicsAPI
	{
		None = 0,
		OpenGL = 1,
		Vulkan = 2
		// DirectX
		// OpenGL ES
	};
	
	// Singletion
	class ConfigManager
	{
	public:
		ConfigManager() {};
		~ConfigManager() {};

		void StartUp(const std::filesystem::path& configFilePath);
		void ShutDown();

		bool LoadConfigFile();
		bool SaveConfigFile();

		// TODO: change config

		inline const std::filesystem::path& GetRootFolder() const { return m_ConfigFilePath; }
		inline const std::filesystem::path& GetAssetFolder() const { return m_AssetPath; }
		inline const std::filesystem::path& GetRegularFontFile() const { return m_RegularFontFile; }
		inline const std::filesystem::path& GetBoldFontFile() const { return m_BoldFontFile; }
		inline const std::filesystem::path& GetFolderIcon() const { return m_FolderIcon; }
		inline const std::filesystem::path& GetFileIcon() const { return m_FileIcon; }
		inline const std::filesystem::path& GetPlayIcon() const { return m_PlayIcon; }
		inline const std::filesystem::path& GetStopIcon() const { return m_StopIcon; }
		inline const std::filesystem::path& GetCacheDir() const { return m_CacheDir; }

		inline GraphicsAPI GetGraphicsAPI() const { return m_GraphicsAPI; }

	private:
		ConfigManager(const ConfigManager&) = delete;
		ConfigManager& operator=(const ConfigManager&) = delete;

		CSimpleIniA m_Ini;

		std::filesystem::path m_ConfigFilePath;

		std::filesystem::path m_AssetPath;

		std::filesystem::path m_BinaryRootFolder;
		std::filesystem::path m_RegularFontFile;
		std::filesystem::path m_BoldFontFile;
		std::filesystem::path m_FolderIcon;
		std::filesystem::path m_FileIcon;
		std::filesystem::path m_PlayIcon;
		std::filesystem::path m_StopIcon;
		std::filesystem::path m_CacheDir;

		// TODO: change in .ini 
		GraphicsAPI m_GraphicsAPI = GraphicsAPI::OpenGL;

	};
}