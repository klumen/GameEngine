#pragma once

#include "runtime/function/render/RHI.h"

extern "C" 
{
	typedef struct GLFWwindow GLFWwindow;
}

namespace Lumen
{
	class OpenGLRHI final : public RHI
	{
	public:
		OpenGLRHI(const RHIInfo& info);
		~OpenGLRHI();

		void SetVSync(bool enabled) override;
		void SetDepthTest(bool enable) override;
		void SetBlend(bool enable) override;
		void SwapBuffers() override;
		void BindTexture(uint32_t slot, uint32_t texture) override;

		std::pair<Shared<Texture>, Shared<Texture>> PreprocessSkybox(AssetHandle skybox) override;

		void RenderShadow(Shared<RenderPipeline> pipeline, const glm::mat4& transform, AssetHandle mesh) override;
		void RenderMesh(Shared<RenderPipeline> pipeline, const glm::mat4& transform, AssetHandle mesh, const std::vector<AssetHandle>& materials) override;
		void RenderLighting(Shared<RenderPipeline> pipeline, Shared<VertexBuffer> VBO, Shared<IndexBuffer> IBO) override;
		void RenderSkybox(Shared<RenderPipeline> pipeline, Shared<VertexBuffer> VBO, Shared<IndexBuffer> IBO, AssetHandle skybox) override;
		void RenderBloom(Shared<RenderPipeline> pipeline, Shared<VertexBuffer> VBO, Shared<IndexBuffer> IBO) override;
		void RenderGaussianBlur(Shared<RenderPipeline> pipeline, Shared<VertexBuffer> VBO, Shared<IndexBuffer> IBO) override;
		void RenderComposite(Shared<RenderPipeline> pipeline, Shared<VertexBuffer> VBO, Shared<IndexBuffer> IBO) override;
		
		void RenderPostProcess(Shared<RenderPipeline> pipeline, Shared<VertexBuffer> VBO, Shared<IndexBuffer> IBO) override;
		void RenderSprites(Shared<RenderPipeline> pipeline, const SpriteBatch& batch) override;

	private:
		GLFWwindow* m_Window{ nullptr };

	};
}