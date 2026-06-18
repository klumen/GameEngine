#include "runtime/Engine.h"

#include "runtime/function/global/GlobalContext.h"
#include "runtime/function/render/Window.h"
#include "runtime/function/render/RenderSystem.h"

namespace Lumen
{
	Engine::Engine()
	{
	}

	Engine::~Engine()
	{
	}

	void Engine::StartUp(const std::filesystem::path& configFilePath)
	{
		GlobalContext::Instance().StartUp(configFilePath);

	}

	void Engine::ShutDown()
	{
		GlobalContext::Instance().ShutDown();
	}
}