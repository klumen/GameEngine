#include "runtime/function/render/RHI.h"

#include "runtime/function/global/GlobalContext.h"
#include "runtime/resource/config/ConfigManager.h"
#include "runtime/core/Macro.h"
#include "runtime/platform/OpenGL/OpenGLRHI.h"

namespace Lumen
{
	Shared<RHI> RHI::Create(const RHIInfo& info)
	{
		switch (LUMEN_GRAPHICS_API)
		{
		case GraphicsAPI::None:    ASSERT(false); return nullptr;
		case GraphicsAPI::OpenGL:  return MakeShared<OpenGLRHI>(info);
		}

		ASSERT(false);
		return nullptr;
	}
}