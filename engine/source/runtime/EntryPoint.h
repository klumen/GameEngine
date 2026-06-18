#pragma once

#include "runtime/Engine.h"
#include "runtime/Application.h"
#include "runtime/function/global/GlobalContext.h"
#include "runtime/core/Log.h"
#include "runtime/core/Macro.h"

#include <locale.h>

int main(int argc, char** argv)
{
	system("CHCP 65001");
	setlocale(LC_ALL, "en_US.UTF-8");

	Lumen::Engine::Instance().StartUp("EditorConfig.ini");

	LUMEN_RUNTIME_INFO("Lumen app start up!");
	Lumen::Application::Instance().StartUp();

	Lumen::Application::Instance().Run();

	LUMEN_RUNTIME_INFO("Lumen app shut down!");
	Lumen::Application::Instance().ShutDown();

	Lumen::Engine::Instance().ShutDown();

	return 0;
}