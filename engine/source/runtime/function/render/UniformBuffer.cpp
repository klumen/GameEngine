#include "runtime/function/render/UniformBuffer.h"

#include "runtime/function/global/GlobalContext.h"
#include "runtime/resource/config/ConfigManager.h"
#include "runtime/core/Macro.h"
#include "runtime/platform/OpenGL/OpenGLUniformBuffer.h"

namespace Lumen
{
	Shared<UniformBuffer> UniformBuffer::Create(uint32_t size, uint32_t binding)
	{
		switch (LUMEN_GRAPHICS_API)
		{
		case GraphicsAPI::None:    ASSERT(false); return nullptr;
		case GraphicsAPI::OpenGL:  return MakeShared<OpenGLUniformBuffer>(size, binding);
		}
		ASSERT(false);
		return nullptr;
	}
}