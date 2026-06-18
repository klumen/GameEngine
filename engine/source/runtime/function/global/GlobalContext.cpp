#include "runtime/function/global/GlobalContext.h"

#include "runtime/core/JobSystem.h"
#include "runtime/core/FileSystem.h"
#include "runtime/core/Log.h"

#include "runtime/resource/config/ConfigManager.h"
#include "runtime/resource/asset/AssetManager.h"

#include "runtime/function/render/Window.h"
#include "runtime/function/render/RenderSystem.h"

namespace Lumen
{
	GlobalContext::GlobalContext()
	{}

	GlobalContext::~GlobalContext()
	{}

	void GlobalContext::StartUp(const std::filesystem::path& configFilePath)
	{
		m_JobSystem = MakeShared<JobSystem>();
		m_JobSystem->StartUp();

		m_FileSystem = MakeShared<FileSystem>();
		m_FileSystem->StartUp();

		m_Log = MakeShared<Log>();
		m_Log->StartUp();

		m_ConfigManager = MakeShared<ConfigManager>();
		m_ConfigManager->StartUp(configFilePath);

		m_AssetManager = MakeShared<AssetManager>();
		m_AssetManager->StartUp();

		m_Window = MakeShared<Window>();
		m_Window->StartUp();

		RenderSystemInfo renderSystemInfo;
		renderSystemInfo.window = m_Window;
		m_RenderSystem = MakeShared<RenderSystem>();
		m_RenderSystem->StartUp(renderSystemInfo);
	}

	void GlobalContext::ShutDown()
	{
		m_RenderSystem->ShutDown();
		m_RenderSystem.reset();

		m_Window->ShutDown();
		m_Window.reset();

		m_AssetManager->ShutDown();
		m_AssetManager.reset();

		m_ConfigManager->ShutDown();
		m_ConfigManager.reset();

		m_Log->ShutDown();
		m_Log.reset();

		m_FileSystem->ShutDown();
		m_FileSystem.reset();

		m_JobSystem->ShutDown();
		m_JobSystem.reset();
	}
}

