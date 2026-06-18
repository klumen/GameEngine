#include "runtime/platform/OpenGL/OpenGLRenderPipeline.h"

#include "runtime/core/Macro.h"

#include <glad/glad.h>

namespace Lumen
{
	static GLenum ShaderDataTypeToOpenGLBaseType(ShaderDataType type)
	{
		switch (type)
		{
		case ShaderDataType::Bool: 			return GL_BOOL;
		case ShaderDataType::Int: 			return GL_INT;
		case ShaderDataType::UInt: 			return GL_UNSIGNED_INT;
		case ShaderDataType::Float: 		return GL_FLOAT;
		case ShaderDataType::Double: 		return GL_DOUBLE;
		case ShaderDataType::Bool2: 		return GL_BOOL;
		case ShaderDataType::Int2: 			return GL_INT;
		case ShaderDataType::UInt2: 		return GL_UNSIGNED_INT;
		case ShaderDataType::Float2: 		return GL_FLOAT;
		case ShaderDataType::Double2: 		return GL_DOUBLE;
		case ShaderDataType::Bool3: 		return GL_BOOL;
		case ShaderDataType::Int3: 			return GL_INT;
		case ShaderDataType::UInt3: 		return GL_UNSIGNED_INT;
		case ShaderDataType::Float3: 		return GL_FLOAT;
		case ShaderDataType::Double3: 		return GL_DOUBLE;
		case ShaderDataType::Bool4: 		return GL_BOOL;
		case ShaderDataType::Int4: 			return GL_INT;
		case ShaderDataType::UInt4: 		return GL_UNSIGNED_INT;
		case ShaderDataType::Float4: 		return GL_FLOAT;
		case ShaderDataType::Double4: 		return GL_DOUBLE;
		case ShaderDataType::Mat2: 			return GL_FLOAT;
		case ShaderDataType::Mat3: 			return GL_FLOAT;
		case ShaderDataType::Mat4: 			return GL_FLOAT;
		case ShaderDataType::DMat2: 		return GL_DOUBLE;
		case ShaderDataType::DMat3: 		return GL_DOUBLE;
		case ShaderDataType::DMat4:			return GL_DOUBLE;
		}

		ASSERT(false);
		return 0;
	}

	OpenGLRenderPipeline::OpenGLRenderPipeline(const RenderPipelineInfo& info)
		: m_Info(info)
	{
		glCreateVertexArrays(1, &m_VertexArrayRendererID);
	}

	OpenGLRenderPipeline::~OpenGLRenderPipeline()
	{
		glDeleteVertexArrays(1, &m_VertexArrayRendererID);
	}

	void OpenGLRenderPipeline::Bind() const
	{
		glBindVertexArray(m_VertexArrayRendererID);

		const auto& layout = m_Info.Layout;
		uint32_t attribIndex = 0;
		for (const auto& element : layout)
		{
			auto glBaseType = ShaderDataTypeToOpenGLBaseType(element.type);
			glEnableVertexAttribArray(attribIndex);
			if (glBaseType == GL_INT)
			{
				glVertexAttribIPointer(attribIndex,
					element.GetComponentCount(),
					glBaseType,
					layout.GetStride(),
					(const void*)(intptr_t)element.offset);
			}
			else if (glBaseType == GL_FLOAT)
			{
				glVertexAttribPointer(attribIndex,
					element.GetComponentCount(),
					glBaseType,
					element.normalized ? GL_TRUE : GL_FALSE,
					layout.GetStride(),
					(const void*)(intptr_t)element.offset);
			}
			else if (glBaseType == GL_DOUBLE)
			{
				glVertexAttribLPointer(attribIndex,
					element.GetComponentCount(),
					glBaseType,
					layout.GetStride(),
					(const void*)(intptr_t)element.offset);
			}
			else if (glBaseType == GL_BOOL)
			{
				glVertexAttribIPointer(attribIndex,
					element.GetComponentCount(),
					glBaseType,
					layout.GetStride(),
					(const void*)(intptr_t)element.offset);
			}
			attribIndex++;
		}
	}

	void OpenGLRenderPipeline::Unbind() const
	{
		glBindVertexArray(0);
	}
}