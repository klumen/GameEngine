#pragma once

#include "runtime/function/render/VertexBuffer.h"
#include "runtime/function/render/IndexBuffer.h"
#include "runtime/resource/asset/Asset.h"
#include "runtime/core/Memory.h"

#include <glm/glm.hpp>

#include <vector>
#include <utility>

namespace Lumen
{
	class Window;
	class RenderPipeline;
	class SpriteBatch;
	class Material;
	class Texture;

	struct RHIInfo
	{
		Shared<Window> window;
	};

	// TODO: add more functions
	// interface
	class RHI
	{
	public:
		virtual ~RHI() = default;

		static Shared<RHI> Create(const RHIInfo& info);

		virtual void SetVSync(bool enabled) = 0;
		virtual void SetDepthTest(bool enable) = 0;
		virtual void SetBlend(bool enable) = 0;
		virtual void SwapBuffers() = 0;
		virtual void BindTexture(uint32_t slot, uint32_t texture) {};
		
		virtual std::pair<Shared<Texture>, Shared<Texture>> PreprocessSkybox(AssetHandle skybox) = 0;

		virtual void RenderShadow(Shared<RenderPipeline> pipeline, const glm::mat4& transform, AssetHandle mesh) = 0;
		virtual void RenderMesh(Shared<RenderPipeline> pipeline, const glm::mat4& transform, AssetHandle mesh, const std::vector<AssetHandle>& materials) = 0;
		virtual void RenderLighting(Shared<RenderPipeline> pipeline, Shared<VertexBuffer> VBO, Shared<IndexBuffer> IBO) = 0;
		virtual void RenderSkybox(Shared<RenderPipeline> pipeline, Shared<VertexBuffer> VBO, Shared<IndexBuffer> IBO, AssetHandle skybox) = 0;
		virtual void RenderBloom(Shared<RenderPipeline> pipeline, Shared<VertexBuffer> VBO, Shared<IndexBuffer> IBO) = 0;
		virtual void RenderGaussianBlur(Shared<RenderPipeline> pipeline, Shared<VertexBuffer> VBO, Shared<IndexBuffer> IBO) = 0;
		virtual void RenderComposite(Shared<RenderPipeline> pipeline, Shared<VertexBuffer> VBO, Shared<IndexBuffer> IBO) = 0;
		
		virtual void RenderPostProcess(Shared<RenderPipeline> pipeline, Shared<VertexBuffer> VBO, Shared<IndexBuffer> IBO) = 0;
		virtual void RenderSprites(Shared<RenderPipeline> pipeline, const SpriteBatch& batch) = 0;

	private:

	};
}