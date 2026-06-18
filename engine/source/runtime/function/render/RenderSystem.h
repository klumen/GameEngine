#pragma once

#include "runtime/resource/asset/Asset.h"
#include "runtime/resource/data/Component.h"
#include "runtime/core/Memory.h"

#include <glm/glm.hpp>

namespace Lumen
{
	class Window;

	struct RenderSystemInfo
	{
		Shared<Window> window;
	};

	class Scene;
	class RHI;

	class RenderSystem
	{
	public:
		RenderSystem() {};
		~RenderSystem() {};

		void StartUp(const RenderSystemInfo& info);
		void ShutDown();

		void Begin();
		void Update();
		void End();

		Shared<RHI> GetRHI() const { return m_RHI; }

		void OnSetViewport(uint32_t width, uint32_t height);
		void OnSetViewProject(const glm::mat4& view, const glm::mat4& project, const glm::vec3& viewPosition);
		void SetSkybox(AssetHandle handle);

		//void Submit();

		void SubmitLight(const LightComponent& light);

		void SubmitMesh(const glm::mat4& transform, const MeshRendererComponent& mesh);
		void SubmitSprite(const glm::mat4& transform, const SpriteRendererComponent& sprite);

		uint32_t GetFinalImageRendererID();

	private:
		RenderSystem(const RenderSystem&) = delete;
		RenderSystem& operator=(const RenderSystem&) = delete;
		
		void SetLights();

		void TestPass();

		void GeometryPass();
		void LightingPass();
		void SkyboxPass();
		void PostProcessPass();
		void CompositePass();

		void ShadowPass();
		void SpritePass();

		Shared<RHI> m_RHI;

	};

}