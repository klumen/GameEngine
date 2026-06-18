#include "runtime/resource/data/Shader.h"

#include "runtime/function/global/GlobalContext.h"
#include "runtime/resource/config/ConfigManager.h"
#include "runtime/core/Macro.h"
#include "runtime/platform/OpenGL/OpenGLShader.h"

namespace Lumen
{
	Shared<Shader> Shader::Create(const ShaderInfo& info)
	{
		switch (LUMEN_GRAPHICS_API)
		{
		case GraphicsAPI::None:    ASSERT(false); return nullptr;
		case GraphicsAPI::OpenGL:  return MakeShared<OpenGLShader>(info);
		}

		ASSERT(false);
		return nullptr;
	}
}