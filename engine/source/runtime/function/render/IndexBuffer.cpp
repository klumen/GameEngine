#include "runtime/function/render/IndexBuffer.h"

#include "runtime/function/global/GlobalContext.h"
#include "runtime/resource/config/ConfigManager.h"
#include "runtime/core/Macro.h"
#include "runtime/platform/OpenGL/OpenGLIndexBuffer.h"
#include "runtime/platform/Vulkan/VulkanIndexBuffer.h"

namespace Lumen
{
	Shared<IndexBuffer> IndexBuffer::Create(const uint32_t* data, uint32_t count)
	{
		switch (LUMEN_GRAPHICS_API)
		{
		case GraphicsAPI::None:    ASSERT(false); return nullptr;
		case GraphicsAPI::OpenGL:  return MakeShared<OpenGLIndexBuffer>(data, count);
		}
		ASSERT(false);
		return nullptr;
	}
}