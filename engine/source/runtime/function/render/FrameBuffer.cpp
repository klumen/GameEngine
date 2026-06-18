#include "runtime/function/render/FrameBuffer.h"

#include "runtime/function/global/GlobalContext.h"
#include "runtime/resource/config/ConfigManager.h"
#include "runtime/core/Macro.h"

#include "runtime/platform/OpenGL/OpenGLFrameBuffer.h"

namespace Lumen
{
	Shared<FrameBuffer> FrameBuffer::Create(const FrameBufferInfo& info)
	{
		switch (LUMEN_GRAPHICS_API)
		{
		case GraphicsAPI::None:    ASSERT(false); return nullptr;
		case GraphicsAPI::OpenGL:  return MakeShared<OpenGLFrameBuffer>(info);
		}

		ASSERT(false);
		return nullptr;
	}
}