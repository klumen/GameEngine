#pragma once

#include "runtime/function/render/VertexBuffer.h"

namespace Lumen
{
	class OpenGLVertexBuffer : public VertexBuffer
	{
	public:
		OpenGLVertexBuffer(const void* data, uint32_t size);
		virtual ~OpenGLVertexBuffer();

		virtual void Bind() const override;
		virtual void Unbind() const override;

		virtual void SetData(const void* data, uint32_t size) const override;

	private:
		uint32_t m_RendererID = 0;

	};
}