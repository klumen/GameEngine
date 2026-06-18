#include "runtime/function/render/ShaderStorageBuffer.h"

#include "runtime/function/global/GlobalContext.h"
#include "runtime/resource/config/ConfigManager.h"
#include "runtime/core/Macro.h"
#include "runtime/platform/OpenGL/OpenGLShaderStorageBuffer.h"

namespace Lumen
{
	Shared<ShaderStorageBuffer> ShaderStorageBuffer::Create()
	{
		switch (LUMEN_GRAPHICS_API)
		{
		case GraphicsAPI::None:    ASSERT(false); return nullptr;
		case GraphicsAPI::OpenGL:  return MakeShared<OpenGLShaderStorageBuffer>();
		}
		ASSERT(false);
		return nullptr;
	}
}