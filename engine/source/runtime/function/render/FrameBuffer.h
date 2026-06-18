#pragma once

#include "runtime/resource/data/Texture.h"
#include "runtime/core/Memory.h"

namespace Lumen
{
	struct FrameBufferAttachmentInfo
	{
		ImageFormat format = ImageFormat::None;
		ImageShape shape = ImageShape::Texture2D;
	};

	struct FrameBufferInfo
	{
		uint32_t Width = 0, Height = 0;
		uint32_t Samples = 1;
		std::vector<FrameBufferAttachmentInfo> Attachments;

		bool SwapChainTarget = false;
	};

	class FrameBuffer
	{
	public:
		virtual ~FrameBuffer() = default;

		static Shared<FrameBuffer> Create(const FrameBufferInfo& info);

		virtual void Bind() = 0;
		virtual void Unbind() = 0;

		virtual void Resize(uint32_t width, uint32_t height) = 0;

		virtual void ClearColor(int drawbuffer, const int* value) = 0;
		virtual void ClearColor(int drawbuffer, const unsigned* value) = 0;
		virtual void ClearColor(int drawbuffer, const float* value) = 0;
		virtual void ClearDepth(const float* value) = 0;
		virtual void ClearStencil(const int* value) = 0;
		virtual void ClearDepthStencil(float depth, int stencil) = 0;

		virtual uint32_t GetDepthAttachmentRendererID() const = 0;
		virtual uint32_t GetColorAttachmentRendererID(uint32_t index = 0) const = 0;

		virtual const FrameBufferInfo& GetInfo() const = 0;

	};
}