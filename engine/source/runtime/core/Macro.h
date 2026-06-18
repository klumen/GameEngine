#pragma once

// other macro
#include "runtime/platform/PlatformDetection.h"
#include "runtime/core/Assert.h"

#define LUMEN_JOB_SYSTEM		::Lumen::GlobalContext::Instance().GetJobSystem()
#define LUMEN_FILE_SYSTEM		::Lumen::GlobalContext::Instance().GetFileSystem()
#define LUMEN_CONFIG_MANAGER	::Lumen::GlobalContext::Instance().GetConfigManager()
#define LUMEN_ASSET_MANAGER		::Lumen::GlobalContext::Instance().GetAssetManager()
#define LUMEN_LOG				::Lumen::GlobalContext::Instance().GetLog()
#define LUMEN_WINDOW			::Lumen::GlobalContext::Instance().GetWindow()
#define LUMEN_RENDER_SYSTEM		::Lumen::GlobalContext::Instance().GetRenderSystem()

#define LUMEN_RUNTIME_TRACE(...)	LUMEN_LOG->GetRuntimeLogger()->trace(__VA_ARGS__)
#define LUMEN_RUNTIME_INFO(...)		LUMEN_LOG->GetRuntimeLogger()->info(__VA_ARGS__)
#define LUMEN_RUNTIME_WARN(...)		LUMEN_LOG->GetRuntimeLogger()->warn(__VA_ARGS__)
#define LUMEN_RUNTIME_ERROR(...)	LUMEN_LOG->GetRuntimeLogger()->error(__VA_ARGS__)
#define LUMEN_RUNTIME_CRITICAL(...)	LUMEN_LOG->GetRuntimeLogger()->critical(__VA_ARGS__)

#define LUMEN_EDITOR_TRACE(...)		LUMEN_LOG->GetEditorLogger()->trace(__VA_ARGS__)
#define LUMEN_EDITOR_INFO(...)		LUMEN_LOG->GetEditorLogger()->info(__VA_ARGS__)
#define LUMEN_EDITOR_WARN(...)		LUMEN_LOG->GetEditorLogger()->warn(__VA_ARGS__)
#define LUMEN_EDITOR_ERROR(...)		LUMEN_LOG->GetEditorLogger()->error(__VA_ARGS__)
#define LUMEN_EDITOR_CRITICAL(...)	LUMEN_LOG->GetEditorLogger()->critical(__VA_ARGS__)

#define LUMEN_GRAPHICS_API LUMEN_CONFIG_MANAGER->GetGraphicsAPI()

#define BIT(x) (1 << x)

#define LUMEN_BIND_EVENT_FN(fn) [this](auto&&... args) -> decltype(auto) { return this->fn(std::forward<decltype(args)>(args)...); }
