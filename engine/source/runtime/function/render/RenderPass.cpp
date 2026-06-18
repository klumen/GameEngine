#include "runtime/function/render/RenderPass.h"

#include "runtime/function/global/GlobalContext.h"
#include "runtime/resource/config/ConfigManager.h"
#include "runtime/core/Macro.h"
#include "runtime/platform/OpenGL/OpenGLRenderPass.h"

namespace Lumen
{
	Shared<RenderPass> RenderPass::Create(const RenderPassInfo& info)
	{
		switch (LUMEN_GRAPHICS_API)
		{
		case GraphicsAPI::None:    ASSERT(false); return nullptr;
		case GraphicsAPI::OpenGL:  return MakeShared<OpenGLRenderPass>(info);
		}

		ASSERT(false);
		return nullptr;
	}
}