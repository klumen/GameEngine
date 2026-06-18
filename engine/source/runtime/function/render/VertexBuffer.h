#pragma once

#include "runtime/resource/data/ShaderDataType.h"
#include "runtime/core/Memory.h"

#include <string>
#include <vector>

namespace Lumen
{
	struct VertexBufferElement
	{
		ShaderDataType type;
		std::string name;
		uint32_t size;
		uint32_t offset;
		bool normalized;

		VertexBufferElement(ShaderDataType type, const std::string& name, bool normalized = false)
			: type(type), name(name), size(ShaderDataTypeSize(type)), offset(0), normalized(normalized)
		{ }

		uint32_t GetComponentCount() const;
	};

	class VertexBufferLayout
	{
	public:
		VertexBufferLayout() {}

		VertexBufferLayout(const std::initializer_list<VertexBufferElement>& elements)
			: m_Elements(elements)
		{
			CalculateOffsetsAndStride();
		}

		uint32_t GetStride() const { return m_Stride; }
		const std::vector<VertexBufferElement>& GetElements() const { return m_Elements; }
		uint32_t GetElementCount() const { return static_cast<uint32_t>(m_Elements.size()); }

		std::vector<VertexBufferElement>::iterator begin() { return m_Elements.begin(); }
		std::vector<VertexBufferElement>::iterator end() { return m_Elements.end(); }
		std::vector<VertexBufferElement>::const_iterator begin() const { return m_Elements.begin(); }
		std::vector<VertexBufferElement>::const_iterator end() const { return m_Elements.end(); }
	
	private:
		void CalculateOffsetsAndStride()
		{
			uint32_t offset = 0;
			m_Stride = 0;
			for (auto& element : m_Elements)
			{
				element.offset = offset;
				offset += element.size;
				m_Stride += element.size;
			}
		}

	private:
		std::vector<VertexBufferElement> m_Elements;
		uint32_t m_Stride = 0;

	};

	enum class VertexBufferUsage
	{
		None = 0, Static = 1, Dynamic = 2
	};

	class VertexBuffer
	{
	public:
		virtual ~VertexBuffer() = default;

		static Shared<VertexBuffer> Create(const void* data, uint32_t size);

		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;

		virtual void SetData(const void* data, uint32_t size) const = 0;

	};
}