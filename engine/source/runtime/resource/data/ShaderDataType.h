#pragma once

#include <cstdint>

namespace Lumen
{
	enum class ShaderDataType : uint8_t
	{
		None = 0,
		Bool, Int, UInt, Float, Double,
		Bool2, Int2, UInt2, Float2, Double2,
		Bool3, Int3, UInt3, Float3, Double3,
		Bool4, Int4, UInt4, Float4, Double4,
		Mat2, Mat3, Mat4,
		DMat2, DMat3, DMat4,
		Sampler1D, Sampler2D, Sampler3D, SamplerCube
	};

	uint32_t ShaderDataTypeSize(ShaderDataType type);
}