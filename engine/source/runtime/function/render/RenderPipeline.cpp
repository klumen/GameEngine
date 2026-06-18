#include "runtime/function/render/RenderPipeline.h"

#include "runtime/function/global/GlobalContext.h"
#include "runtime/resource/config/ConfigManager.h"
#include "runtime/core/Macro.h"
#include "runtime/platform/OpenGL/OpenGLRenderPipeline.h"

namespace Lumen
{
	Shared<RenderPipeline> RenderPipeline::Create(const RenderPipelineInfo& info)
	{
		switch (LUMEN_GRAPHICS_API)
		{
		case GraphicsAPI::None:    ASSERT(false); return nullptr;
		case GraphicsAPI::OpenGL:  return MakeShared<OpenGLRenderPipeline>(info);
		}

		ASSERT(false);
		return nullptr;
	}
}