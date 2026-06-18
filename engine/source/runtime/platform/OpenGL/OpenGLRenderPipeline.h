#pragma once

#include "runtime/function/render/RenderPipeline.h"

namespace Lumen
{
	class OpenGLRenderPipeline : public RenderPipeline
	{
	public:
		OpenGLRenderPipeline(const RenderPipelineInfo& info);
		virtual ~OpenGLRenderPipeline();

		virtual RenderPipelineInfo& GetInfo() override { return m_Info; }
		
		virtual void Bind() const override;
		virtual void Unbind() const override;

	private:
		RenderPipelineInfo m_Info;

		uint32_t m_VertexArrayRendererID = 0;

	};
}