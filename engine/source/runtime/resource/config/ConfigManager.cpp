#include "runtime/resource/config/ConfigManager.h"

#include "runtime/function/global/GlobalContext.h"
#include "runtime/core/Log.h"
#include "runtime/core/Macro.h"

namespace Lumen
{
	void ConfigManager::StartUp(const std::filesystem::path& configFilePath)
	{
		m_ConfigFilePath = configFilePath;
		bool isLoaded = LoadConfigFile();
		ASSERT(isLoaded);
	}

	void ConfigManager::ShutDown()
	{
		 SaveConfigFile();
	}

	bool ConfigManager::LoadConfigFile()
	{
		m_Ini.SetUnicode(true);
		m_Ini.SetMultiKey(false);

		SI_Error rc = m_Ini.LoadFile(m_ConfigFilePath.c_str());
		if (rc < 0)
		{
			LUMEN_RUNTIME_ERROR("Failed to load .ini file: {}", m_ConfigFilePath.string());
			return false;
		}
		
		m_BinaryRootFolder = m_ConfigFilePath.parent_path() / m_Ini.GetValue("EditorConfig", "BinaryRootFolder");
		CSimpleIniA::TNamesDepend sections;
		m_Ini.GetAllSections(sections);
		for (const auto& it : sections) 
		{
			const CSimpleIniA::TKeyVal* pKeyVal = m_Ini.GetSection(it.pItem);
			if (!pKeyVal) continue;
			for (const auto& it : *pKeyVal) 
			{
				if (std::string_view(it.first.pItem) == "AssetFolder")
				{
					ASSERT(!m_BinaryRootFolder.empty());
					m_AssetPath = m_BinaryRootFolder / it.second;
				}
				else if (std::string_view(it.first.pItem) == "RegularFont")
				{
					ASSERT(!m_BinaryRootFolder.empty());
					m_RegularFontFile = m_BinaryRootFolder / it.second;
				}
				else if (std::string_view(it.first.pItem) == "BoldFont")
				{
					ASSERT(!m_BinaryRootFolder.empty());
					m_BoldFontFile = m_BinaryRootFolder / it.second;
				}
				else if (std::string_view(it.first.pItem) == "FolderIcon")
				{
					ASSERT(!m_BinaryRootFolder.empty());
					m_FolderIcon = m_BinaryRootFolder / it.second;
				}
				else if (std::string_view(it.first.pItem) == "FileIcon")
				{
					ASSERT(!m_BinaryRootFolder.empty());
					m_FileIcon = m_BinaryRootFolder / it.second;
				}
				else if (std::string_view(it.first.pItem) == "PlayIcon")
				{
					ASSERT(!m_BinaryRootFolder.empty());
					m_PlayIcon = m_BinaryRootFolder / it.second;
				}
				else if (std::string_view(it.first.pItem) == "StopIcon")
				{
					ASSERT(!m_BinaryRootFolder.empty());
					m_StopIcon = m_BinaryRootFolder / it.second;
				}
				else if (std::string_view(it.first.pItem) == "CacheDir")
				{
					ASSERT(!m_BinaryRootFolder.empty());
					m_CacheDir = m_BinaryRootFolder / it.second;
				}
			}
		}

		return true;

		// maybe a better way
		/*std::unordered_map<std::string_view, std::function<void(const auto&)>> handlers;

		handlers["BinaryRootFolder"] = [this](constauto& it) {
			m_BinaryRootFolder = m_ConfigFilePath.parent_path() / it.second;
			};

		handlers["RegularFontFile"] = [this](constauto& it) {
			assert(!m_BinaryRootFolder.empty());
			m_RegularFontFile = m_BinaryRootFolder / it.second;
			};

		handlers["BoldFontFile"] = [this](constauto& it) {
			assert(!m_BinaryRootFolder.empty());
			m_BoldFontFile = m_BinaryRootFolder / it.second;
			};

		for (const auto& it : *pKeyVal) {
			auto handler = handlers.find(std::string_view(it.first.pItem));
			if (handler != handlers.end()) {
				handler->second(it);
			}
		}*/
	}

	// TODO: add change the .ini file function

	bool ConfigManager::SaveConfigFile()
	{
		SI_Error rc = m_Ini.SaveFile(m_ConfigFilePath.c_str());
		if (rc < 0) 
		{
			LUMEN_RUNTIME_ERROR("Failed to save .ini file: {}", m_ConfigFilePath.string());
			return false;
		}

		return true;
	}
}