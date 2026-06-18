#include "runtime/function/render/VertexBuffer.h"

#include "runtime/function/global/GlobalContext.h"
#include "runtime/resource/config/ConfigManager.h"
#include "runtime/core/Macro.h"
#include "runtime/platform/OpenGL/OpenGLVertexBuffer.h"

namespace Lumen
{
	Shared<VertexBuffer> VertexBuffer::Create(const void* data, uint32_t size)
	{
		switch(LUMEN_GRAPHICS_API)
		{
		case GraphicsAPI::None:    ASSERT(false); return nullptr;
		case GraphicsAPI::OpenGL:  return MakeShared<OpenGLVertexBuffer>(data, size);
		}

		ASSERT(false);
		return nullptr;
	}

	uint32_t VertexBufferElement::GetComponentCount() const
	{
		switch (type)
		{
		case ShaderDataType::Bool:    return 1;
		case ShaderDataType::Int:     return 1;
		case ShaderDataType::UInt:    return 1;
		case ShaderDataType::Float:   return 1;
		case ShaderDataType::Double:  return 1;
		case ShaderDataType::Bool2:   return 2;
		case ShaderDataType::Int2:    return 2;
		case ShaderDataType::UInt2:   return 2;
		case ShaderDataType::Float2:  return 2;
		case ShaderDataType::Double2: return 2;
		case ShaderDataType::Bool3:   return 3;
		case ShaderDataType::Int3:    return 3;
		case ShaderDataType::UInt3:   return 3;
		case ShaderDataType::Float3:  return 3;
		case ShaderDataType::Double3: return 3;
		case ShaderDataType::Bool4:   return 4;
		case ShaderDataType::Int4:    return 4;
		case ShaderDataType::UInt4:   return 4;
		case ShaderDataType::Float4:  return 4;
		case ShaderDataType::Double4: return 4;
		case ShaderDataType::Mat2:    return 2 * 2;
		case ShaderDataType::Mat3:    return 3 * 3;
		case ShaderDataType::Mat4:    return 4 * 4;
		case ShaderDataType::DMat2:   return 2 * 2;
		case ShaderDataType::DMat3:   return 3 * 3;
		case ShaderDataType::DMat4:   return 4 * 4;
		}

		ASSERT(false);
		return 0;
	}
}