#pragma once

#include "runtime/function/render/FrameBuffer.h"

namespace Lumen
{
	struct RenderPassInfo
	{
		Shared<FrameBuffer> Target;
	};

	class RenderPass
	{
	public:
		virtual ~RenderPass() = default;

		static Shared<RenderPass> Create(const RenderPassInfo& info);

		virtual RenderPassInfo& GetInfo() = 0;
	};
}