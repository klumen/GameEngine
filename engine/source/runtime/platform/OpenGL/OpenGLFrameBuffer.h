#pragma once

#include "runtime/function/render/FrameBuffer.h"
#include "runtime/core/Macro.h"

#include <cassert>

namespace Lumen
{
	class OpenGLFrameBuffer : public FrameBuffer
	{
	public:
		OpenGLFrameBuffer(const FrameBufferInfo& info);
		virtual ~OpenGLFrameBuffer();

		void Flush();

		virtual void Bind() override;
		virtual void Unbind() override;

		virtual void Resize(uint32_t width, uint32_t height) override;

		virtual void ClearColor(int drawbuffer, const int* value) override;
		virtual void ClearColor(int drawbuffer, const unsigned* value) override;
		virtual void ClearColor(int drawbuffer, const float* value) override;
		virtual void ClearDepth(const float* value) override;
		virtual void ClearStencil(const int* value) override;
		virtual void ClearDepthStencil(float depth, int stencil) override;

		virtual uint32_t GetDepthAttachmentRendererID() const override
		{
			return m_DepthAttachment;
		}
		virtual uint32_t GetColorAttachmentRendererID(uint32_t index = 0) const override
		{
			ASSERT(index < m_ColorAttachments.size()); 
			return m_ColorAttachments[index];
		}

		virtual const FrameBufferInfo& GetInfo() const override { return m_Info; }

	private:
		uint32_t m_RendererID = 0;
		FrameBufferInfo m_Info;

		std::vector<FrameBufferAttachmentInfo> m_ColorAttaInfo;
		FrameBufferAttachmentInfo m_DepthAttaInfo;

		std::vector<uint32_t> m_ColorAttachments;
		uint32_t m_DepthAttachment = 0;

	};
}