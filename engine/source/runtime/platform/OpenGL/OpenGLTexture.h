#pragma once

#include "runtime/resource/data/Texture.h"

namespace Lumen
{
	class OpenGLTexture : public Texture
	{
	public:
		OpenGLTexture(const TextureInfo& info, const std::vector<uint8_t>& data);
		OpenGLTexture(const TextureInfo& info);
		virtual ~OpenGLTexture();

		virtual uint32_t GetWidth() const override { return m_Info.width; }
		virtual uint32_t GetHeight() const override { return m_Info.height; }
		virtual ImageFormat GetFormat() const override { return m_Info.format; }
		virtual ImageShape GetShape() const override { return m_Info.shape; }
		virtual uint32_t GetMipLevelCount() const override;
		virtual uint32_t GetRendererID() const override { return m_RendererID; }

		virtual void Bind(uint32_t slot) const override;
		virtual void SetData(const void* data) const override;

	private:
		uint32_t m_RendererID = 0;
		
		TextureInfo m_Info{};

	};
}