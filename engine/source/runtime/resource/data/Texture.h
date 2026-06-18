#pragma once

#include "runtime/resource/asset/Asset.h"
#include "runtime/core/Memory.h"

#include <vector>
#include <variant>

namespace Lumen
{
	enum class ImageShape
	{
		None = 0, 
		Texture2D, 
		Texture2DArray, 
		Cubemap, 
		CubemapArray, 
		Texture3D
	};

	enum class ImageFormat
	{
		None = 0,

		// Color
		R32I,
		R32UI,
		R8,
		RG8,
		RGB8,
		RGBA8,
		SRGB8,
		SRGBA8,
		R16F,
		RG16F,
		RGB16F,
		RGBA16F,
		R32F,
		RG32F,
		RGB32F,
		RGBA32F,
		RGB10_A2,

		// Depth/Stencil
		DEPTH24_STENCIL8,
		DEPTH32F,
		DEPTH32F_STENCIL8
	};

	struct TextureInfo
	{
		uint32_t width = 1;
		uint32_t height = 1;
		ImageFormat format = ImageFormat::RGBA8;
		ImageShape shape = ImageShape::Texture2D;

		bool generateMipmaps = false;
		bool isHDR = false;
	};

	class Texture : public Asset
	{
	public:
		virtual ~Texture() = default;

		static AssetType GetStaticType() { return AssetType::Texture; }
		virtual AssetType GetType() const override { return GetStaticType(); };

		static Shared<Texture> Create(const TextureInfo& info, const std::vector<uint8_t>& data);
		static Shared<Texture> Create(const TextureInfo& info);
		
		virtual uint32_t GetWidth() const = 0;
		virtual uint32_t GetHeight() const = 0;
		virtual ImageFormat GetFormat() const = 0;
		virtual ImageShape GetShape() const = 0;
		virtual uint32_t GetMipLevelCount() const = 0;
		virtual uint32_t GetRendererID() const = 0;

		virtual void Bind(uint32_t slot) const = 0;

		virtual void SetData(const void* data) const = 0;

	};

	// maybe TODO: Class Texture2D/Cubemap : public Texture
}