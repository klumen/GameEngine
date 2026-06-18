#include "runtime/platform/OpenGL/OpenGLTexture.h"

#include "runtime/function/global/GlobalContext.h"
#include "runtime/function/render/RenderSystem.h"
#include "runtime/resource/asset/AssetManager.h"
#include "runtime/core/Macro.h"

#include <glad/glad.h>

#include <cassert>

namespace Lumen
{
	static GLenum ImageFormatToGLInternalFormat(ImageFormat format)
	{
		switch (format)
		{
		case ImageFormat::R8:		return GL_R8;
		case ImageFormat::RG8:		return GL_RG8;
		case ImageFormat::RGB8:		return GL_RGB8;
		case ImageFormat::RGBA8:	return GL_RGBA8;
		case ImageFormat::SRGB8:	return GL_SRGB8;
		case ImageFormat::SRGBA8:	return GL_SRGB8_ALPHA8;
		case ImageFormat::R16F:		return GL_R16F;
		case ImageFormat::RG16F:	return GL_RG16F;
		case ImageFormat::RGB16F:	return GL_RGB16F;
		case ImageFormat::RGBA16F:	return GL_RGBA16F;
		case ImageFormat::R32F:		return GL_R32F;
		case ImageFormat::RG32F:	return GL_RG32F;
		case ImageFormat::RGB32F:	return GL_RGB32F;
		case ImageFormat::RGBA32F:	return GL_RGBA32F;
		}

		ASSERT(false);
		return 0;
	}

	// TODO: GL_DEPTH_COMPONENT, and GL_STENCIL_INDEX.
	static GLenum ImageFormatToGLDataFormat(ImageFormat format)
	{
		switch (format)
		{
		case ImageFormat::R8:		return GL_RED;
		case ImageFormat::RG8:		return GL_RG;
		case ImageFormat::RGB8:		return GL_RGB;
		case ImageFormat::RGBA8:	return GL_RGBA;
		case ImageFormat::SRGB8:	return GL_RGB;
		case ImageFormat::SRGBA8:	return GL_RGBA;
		case ImageFormat::R16F:		return GL_RED;
		case ImageFormat::RG16F:	return GL_RG;
		case ImageFormat::RGB16F:	return GL_RGB;
		case ImageFormat::RGBA16F:	return GL_RGBA;
		case ImageFormat::R32F:		return GL_RED;
		case ImageFormat::RG32F:	return GL_RG;
		case ImageFormat::RGB32F:	return GL_RGB;
		case ImageFormat::RGBA32F:	return GL_RGBA;
		}

		ASSERT(false);
		return 0;
	}

	OpenGLTexture::OpenGLTexture(const TextureInfo& info, const std::vector<uint8_t>& data)
		: m_Info(info)
	{
		GLenum internalFormat = ImageFormatToGLInternalFormat(m_Info.format);
		GLenum dataFormat = ImageFormatToGLDataFormat(m_Info.format);

		if (m_Info.shape == ImageShape::Texture2D)
		{
			uint32_t levels = m_Info.generateMipmaps ? GetMipLevelCount() : 1;

			glCreateTextures(GL_TEXTURE_2D, 1, &m_RendererID);
			glTextureStorage2D(m_RendererID, levels, internalFormat, m_Info.width, m_Info.height);

			glTextureParameteri(m_RendererID, GL_TEXTURE_MIN_FILTER, m_Info.generateMipmaps ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
			glTextureParameteri(m_RendererID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_T, GL_REPEAT);

			if (m_Info.isHDR)
			{
				glTextureSubImage2D(m_RendererID, 0, 0, 0, m_Info.width, m_Info.height, dataFormat, GL_FLOAT, data.data());
			}
			else
			{
				glTextureSubImage2D(m_RendererID, 0, 0, 0, m_Info.width, m_Info.height, dataFormat, GL_UNSIGNED_BYTE, data.data());
			}
		}
		else if (m_Info.shape == ImageShape::Cubemap)
		{
			TextureInfo tempInfo = m_Info;
			tempInfo.generateMipmaps = true;
			tempInfo.shape = ImageShape::Texture2D;
			auto texture = OpenGLTexture(tempInfo, data);

			m_Info.width = m_Info.height = std::min(m_Info.height, m_Info.width);
			uint32_t levels = m_Info.generateMipmaps ? GetMipLevelCount() : 1;

			glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &m_RendererID);
			glTextureStorage2D(m_RendererID, levels, internalFormat, m_Info.width, m_Info.height);
			
			glTextureParameteri(m_RendererID, GL_TEXTURE_MIN_FILTER, m_Info.generateMipmaps ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
			glTextureParameteri(m_RendererID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
			
			auto shader = LUMEN_ASSET_MANAGER->GetRuntimeAsset<Shader>("EquirectangularToCubeMapShader");
			shader->Bind();
			texture.Bind(1);

			glBindImageTexture(0, m_RendererID, 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA16F);
			glDispatchCompute(m_Info.width / 32, m_Info.height / 32, 6);
			glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT | GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
		}

		if (m_Info.generateMipmaps)
			glGenerateTextureMipmap(m_RendererID);
	}

	OpenGLTexture::OpenGLTexture(const TextureInfo& info)
		: m_Info(info)
	{
		GLenum internalFormat = ImageFormatToGLInternalFormat(m_Info.format);
		GLenum dataFormat = ImageFormatToGLDataFormat(m_Info.format);

		uint32_t levels = m_Info.generateMipmaps ? GetMipLevelCount() : 1;

		if (m_Info.shape == ImageShape::Texture2D)
		{
			glCreateTextures(GL_TEXTURE_2D, 1, &m_RendererID);
			glTextureStorage2D(m_RendererID, levels, internalFormat, m_Info.width, m_Info.height);

			glTextureParameteri(m_RendererID, GL_TEXTURE_MIN_FILTER, m_Info.generateMipmaps ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
			glTextureParameteri(m_RendererID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_T, GL_REPEAT);
		}
		else if (m_Info.shape == ImageShape::Cubemap)
		{
			glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &m_RendererID);
			glTextureStorage2D(m_RendererID, levels, internalFormat, m_Info.width, m_Info.height);

			glTextureParameteri(m_RendererID, GL_TEXTURE_MIN_FILTER, m_Info.generateMipmaps ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
			glTextureParameteri(m_RendererID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		}
	}

	OpenGLTexture::~OpenGLTexture()
	{
		glDeleteTextures(1, &m_RendererID);
	}

	uint32_t OpenGLTexture::GetMipLevelCount() const
	{
		return static_cast<uint32_t>(std::floor(std::log2(std::min(m_Info.width, m_Info.height))) + 1);
	}

	void OpenGLTexture::Bind(uint32_t slot) const
	{
		glBindTextureUnit(slot, m_RendererID);
	}

	void OpenGLTexture::SetData(const void* data) const
	{
		GLenum dataFormat = ImageFormatToGLDataFormat(m_Info.format);
		GLenum dataType = m_Info.isHDR ? GL_FLOAT : GL_UNSIGNED_BYTE;
		glTextureSubImage2D(m_RendererID, 0, 0, 0, m_Info.width, m_Info.height, dataFormat, dataType, data);
		if (m_Info.generateMipmaps)
			glGenerateTextureMipmap(m_RendererID);
	}
}