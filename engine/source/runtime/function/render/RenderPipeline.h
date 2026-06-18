#pragma once

#include "runtime/function/render/RenderPass.h"
#include "runtime/function/render/VertexBuffer.h" 
#include "runtime/resource/data/Shader.h"

namespace Lumen
{
	struct RenderPipelineInfo
	{
		Shared<Shader> Shader;
		VertexBufferLayout Layout;
		Shared<RenderPass> RenderPass;
	};

	class RenderPipeline
	{
	public:
		virtual ~RenderPipeline() = default;

		static Shared<RenderPipeline> Create(const RenderPipelineInfo& info);

		virtual RenderPipelineInfo& GetInfo() = 0;

		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;

	};
}