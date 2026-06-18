#pragma once

#include "runtime/function/render/RenderPass.h"

namespace Lumen
{
	class OpenGLRenderPass : public RenderPass
	{
	public:
		OpenGLRenderPass(const RenderPassInfo& info);
		virtual ~OpenGLRenderPass() = default;

		virtual RenderPassInfo& GetInfo() override { return m_Info; };

	private:
		RenderPassInfo m_Info;

	};
}