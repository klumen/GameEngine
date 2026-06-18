#include "runtime/resource/data/Texture.h"

#include "runtime/function/global/GlobalContext.h"
#include "runtime/resource/config/ConfigManager.h"
#include "runtime/core/Macro.h"
#include "runtime/platform/OpenGL/OpenGLTexture.h"

namespace Lumen
{
	Shared<Texture> Texture::Create(const TextureInfo& info, const std::vector<uint8_t>& data)
	{
		switch (LUMEN_GRAPHICS_API)
		{
		case GraphicsAPI::None:    ASSERT(false); return nullptr;
		case GraphicsAPI::OpenGL:  return MakeShared<OpenGLTexture>(info, data);
		}

		ASSERT(false);
		return nullptr;
	}

	Shared<Texture> Texture::Create(const TextureInfo& info)
	{
		switch (LUMEN_GRAPHICS_API)
		{
		case GraphicsAPI::None:    ASSERT(false); return nullptr;
		case GraphicsAPI::OpenGL:  return MakeShared<OpenGLTexture>(info);
		}

		ASSERT(false);
		return nullptr;
	}
}