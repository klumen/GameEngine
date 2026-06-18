#pragma once

#include "runtime/function/render/IndexBuffer.h"

namespace Lumen
{
	class VulkanIndexBuffer : public IndexBuffer
	{
	public:
		VulkanIndexBuffer(const uint32_t* data, uint32_t count);
		~VulkanIndexBuffer();

		virtual void Bind() const override;
		virtual void Unbind() const override;
		virtual uint32_t GetCount() const override { return m_Count; }

	private:
		uint32_t m_Count = 0;

	};
}