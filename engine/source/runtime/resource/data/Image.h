#pragma once

#include "runtime/core/Memory.h"

#include <vector>

namespace Lumen
{
	//enum class ImageShape : uint8_t
	//{
	//	None = 0,
	//	Texture2D, CubeMap, Texture3D
	//};

	//enum class ImageFormat : uint8_t
	//{
	//	None = 0,

	//	// Color
	//	R32I,
	//	R32UI,
	//	R8,
	//	RG8,
	//	RGB8,
	//	RGBA8,
	//	SRGB8,
	//	SRGBA8,
	//	R16F,
	//	RG16F,
	//	RGB16F,
	//	RGBA16F,
	//	R32F,
	//	RG32F,
	//	RGB32F,
	//	RGBA32F,

	//	// Depth/Stencil
	//	DEPTH24STENCIL8,
	//	DEPTH32F,
	//};

	//struct TextureInfo
	//{
	//	uint32_t width = 1;
	//	uint32_t height = 1;
	//	ImageFormat format = ImageFormat::RGBA8;

	//	bool generateMipmaps = true;
	//	bool isHDR = false;
	//};

	class Image
	{
	public:
		virtual ~Image() = default;

		static Shared<Image> CreateImage();

	private:

	};
}