#include "runtime/function/render/RenderSystem.h"

#include "runtime/function/global/GlobalContext.h"
#include "runtime/function/render/RHI.h"
#include "runtime/function/render/SpriteBatch.h"
#include "runtime/function/render/FrameBuffer.h"
#include "runtime/function/render/UniformBuffer.h"
#include "runtime/function/render/RenderPipeline.h"
#include "runtime/resource/asset/AssetManager.h"
#include "runtime/resource/asset/ShaderImporter.h"
#include "runtime/resource/asset/TextureImporter.h"
#include "runtime/resource/data/Shader.h"
#include "runtime/resource/data/Scene.h"
#include "runtime/resource/data/Entity.h"
#include "runtime/resource/data/Component.h"
#include "runtime/core/Macro.h"

#include <glm/gtc/type_ptr.hpp>

namespace Lumen
{
	static float s_QuadVertices[] = {
		// 位置                  // 纹理坐标  
		-1.0f, -1.0f, 0.0f,     0.0f, 0.0f,  // 左下角  
		 1.0f, -1.0f, 0.0f,     1.0f, 0.0f,  // 右下角  
		-1.0f,  1.0f, 0.0f,     0.0f, 1.0f,  // 左上角  
		 1.0f,  1.0f, 0.0f,     1.0f, 1.0f   // 右上角  
	};

	unsigned int s_QuadIndices[] = {
	0, 1, 2,  // 第一个三角形  
	1, 3, 2   // 第二个三角形  
	};

	struct RenderData
	{
		uint32_t ViewportWidth = 1920;
		uint32_t ViewportHeight = 1080;

		glm::mat4 View = glm::mat4(1.f);
		glm::mat4 Projection = glm::mat4(1.f);
		glm::vec3 viewPosition{ 0.f };
		//Shared<UniformBuffer> VPUBO;

		Shared<VertexBuffer> QuadVBO;
		Shared<IndexBuffer> QuadIBO;

		Shared<RenderPipeline> TestPipeline;

		Shared<RenderPipeline> GeometryPipeline;
		Shared<RenderPipeline> LightingPipeline;
		Shared<RenderPipeline> SkyboxPipeline;
		Shared<RenderPipeline> BloomPipeline;
		Shared<RenderPipeline> GaussianBlurPipeline;
		Shared<RenderPipeline> PostProcessPipeline;
		Shared<RenderPipeline> CompositePipeline;

		Shared<RenderPipeline> DirectionalShadowPipeline;
		Shared<RenderPipeline> SpotShadowPipeline;
		Shared<RenderPipeline> PointShadowPipeline;
		Shared<RenderPipeline> SpritePipeline;
	};

	struct LightingData
	{
		static constexpr uint32_t MAX_LIGHT_NUM = 5;
		//Shared<UniformBuffer> LightUBO;

		AssetHandle Skybox = 0;
		Shared<Texture> SkyboxPrefiltered;
		Shared<Texture> SkyboxIrradiance;
	};

	static RenderData s_Data;
	static LightingData s_LightingData;

	static SpriteBatch s_SpriteBatch;

	static std::vector<LightComponent> s_Lights;

	static std::vector<std::pair<glm::mat4, MeshRendererComponent>> meshDrawList;
	static std::vector<std::pair<glm::mat4, SpriteRendererComponent>> spriteDrawList;

	void RenderSystem::StartUp(const RenderSystemInfo& info)
	{
		RHIInfo rhiInfo;
		rhiInfo.window = info.window;
		m_RHI = RHI::Create(rhiInfo);

		// TODO: in-game option -- .ini
		m_RHI->SetVSync(0);

		/*s_Data.VPUBO = UniformBuffer::Create(64 * 2, 0);
		s_Data.LightUBO = UniformBuffer::Create(640, 1);*/

		TextureInfo whiteTextureInfo{};
		
		Shared<Texture> WhiteTexture = Texture::Create(whiteTextureInfo);
		constexpr uint32_t whiteTextureData = 0xffffffff;
		WhiteTexture->SetData(&whiteTextureData);
		LUMEN_ASSET_MANAGER->UpdateRuntimeAsset("WhiteTexture", WhiteTexture);

		s_SpriteBatch.textureSlots[0] = WhiteTexture;

		s_SpriteBatch.VBO = VertexBuffer::Create(nullptr, SpriteBatch::MaxVertices * sizeof(SpriteVertex));

		std::vector<uint32_t> spritesIndices(SpriteBatch::MaxIndices);
		uint32_t offset = 0;
		for (uint32_t i = 0; i < SpriteBatch::MaxIndices; i += 6)
		{
			spritesIndices[i + 0ull] = offset + 0;
			spritesIndices[i + 1ull] = offset + 1;
			spritesIndices[i + 2ull] = offset + 2;

			spritesIndices[i + 3ull] = offset + 2;
			spritesIndices[i + 4ull] = offset + 3;
			spritesIndices[i + 5ull] = offset + 0;

			offset += 4;
		}
		s_SpriteBatch.IBO = IndexBuffer::Create(spritesIndices.data(), SpriteBatch::MaxIndices);

		s_Data.QuadVBO = VertexBuffer::Create(s_QuadVertices, sizeof(s_QuadVertices));
		s_Data.QuadIBO = IndexBuffer::Create(s_QuadIndices, 6);

		{
			TextureImporter importer("resource/textures/BRDF_LUT.tga");
			auto texture = std::reinterpret_pointer_cast<Texture>(importer.Import());
			LUMEN_ASSET_MANAGER->UpdateRuntimeAsset("BRDFLUTTexture", texture);
		}

		/*{
			ShaderImporter importer("resource/shaders/Test.glsl");
			auto shader = std::reinterpret_pointer_cast<Shader>(importer.Import());
			LUMEN_ASSET_MANAGER->UpdateRuntimeAsset("TestShader", shader);
		}*/

		{
			ShaderImporter importer("resource/shaders/PointShadow.glsl");
			auto shader = std::reinterpret_pointer_cast<Shader>(importer.Import());
			LUMEN_ASSET_MANAGER->UpdateRuntimeAsset("PointShadowShader", shader);
		}

		{
			ShaderImporter importer("resource/shaders/EquirectangularToCubeMap.glsl");
			auto shader = std::reinterpret_pointer_cast<Shader>(importer.Import());
			LUMEN_ASSET_MANAGER->UpdateRuntimeAsset("EquirectangularToCubeMapShader", shader);
		}

		{
			ShaderImporter importer("resource/shaders/SkyboxIrradiance.glsl");
			auto shader = std::reinterpret_pointer_cast<Shader>(importer.Import());
			LUMEN_ASSET_MANAGER->UpdateRuntimeAsset("SkyboxIrradianceShader", shader);
		}

		{
			ShaderImporter importer("resource/shaders/SkyboxPrefilter.glsl");
			auto shader = std::reinterpret_pointer_cast<Shader>(importer.Import());
			LUMEN_ASSET_MANAGER->UpdateRuntimeAsset("SkyboxPrefilterShader", shader);
		}

		{
			ShaderImporter importer("resource/shaders/PBRStatic.glsl");
			auto shader = std::reinterpret_pointer_cast<Shader>(importer.Import());
			LUMEN_ASSET_MANAGER->UpdateRuntimeAsset("PBRStaticShader", shader);
		}

		{
			ShaderImporter importer("resource/shaders/Lighting.glsl");
			auto shader = std::reinterpret_pointer_cast<Shader>(importer.Import());
			LUMEN_ASSET_MANAGER->UpdateRuntimeAsset("LightingShader", shader);
		}

		{
			ShaderImporter importer("resource/shaders/Skybox.glsl");
			auto shader = std::reinterpret_pointer_cast<Shader>(importer.Import());
			LUMEN_ASSET_MANAGER->UpdateRuntimeAsset("SkyboxShader", shader);
		}

		{
			ShaderImporter importer("resource/shaders/Bloom.glsl");
			auto shader = std::reinterpret_pointer_cast<Shader>(importer.Import());
			LUMEN_ASSET_MANAGER->UpdateRuntimeAsset("BloomShader", shader);
		}

		{
			ShaderImporter importer("resource/shaders/GaussianBlur.glsl");
			auto shader = std::reinterpret_pointer_cast<Shader>(importer.Import());
			LUMEN_ASSET_MANAGER->UpdateRuntimeAsset("GaussianBlurShader", shader);
		}

		{
			ShaderImporter importer("resource/shaders/Composite.glsl");
			auto shader = std::reinterpret_pointer_cast<Shader>(importer.Import());
			LUMEN_ASSET_MANAGER->UpdateRuntimeAsset("CompositeShader", shader);
		}

		/*{
			FrameBufferInfo FBOInfo;
			FBOInfo.Width = 1920;
			FBOInfo.Height = 1080;
			FBOInfo.Attachments = {
				{ ImageFormat::RGBA16F },
				{ ImageFormat::DEPTH24_STENCIL8 } };
			FBOInfo.Samples = 1;
			Shared<FrameBuffer> FBO = FrameBuffer::Create(FBOInfo);

			RenderPipelineInfo pipelineInfo;
			pipelineInfo.Layout = {
				{ ShaderDataType::Float3, "a_Position" },
				{ ShaderDataType::Float3, "a_Normal" },
				{ ShaderDataType::Float3, "a_Tangent" },
				{ ShaderDataType::Float3, "a_Binormal" },
				{ ShaderDataType::Float2, "a_TexCoord" },
			};
			pipelineInfo.Shader = LUMEN_ASSET_MANAGER->GetRuntimeAsset<Shader>("TestShader");

			RenderPassInfo renderPassInfo;
			renderPassInfo.Target = FBO;
			pipelineInfo.RenderPass = RenderPass::Create(renderPassInfo);
			s_Data.TestPipeline = RenderPipeline::Create(pipelineInfo);
		}*/

		{
			FrameBufferInfo FBOInfo;
			FBOInfo.Width = 512;
			FBOInfo.Height = 512;
			FBOInfo.Attachments = {
				{ ImageFormat::DEPTH32F, ImageShape::Cubemap } };
			FBOInfo.Samples = 1;
			Shared<FrameBuffer> FBO = FrameBuffer::Create(FBOInfo);

			RenderPipelineInfo pipelineInfo;
			pipelineInfo.Layout = {
				{ ShaderDataType::Float3, "a_Position" },
				{ ShaderDataType::Float3, "a_Normal" },
				{ ShaderDataType::Float3, "a_Tangent" },
				{ ShaderDataType::Float3, "a_Binormal" },
				{ ShaderDataType::Float2, "a_TexCoord" },
			};
			pipelineInfo.Shader = LUMEN_ASSET_MANAGER->GetRuntimeAsset<Shader>("PointShadowShader");

			RenderPassInfo renderPassInfo;
			renderPassInfo.Target = FBO;
			pipelineInfo.RenderPass = RenderPass::Create(renderPassInfo);
			s_Data.PointShadowPipeline = RenderPipeline::Create(pipelineInfo);
		}

		{
			FrameBufferInfo FBOInfo;
			FBOInfo.Width = 1920;
			FBOInfo.Height = 1080;
			FBOInfo.Attachments = { 
				{ ImageFormat::RGBA32F }, // (world)position(RGB), unused(A)
				{ ImageFormat::RGBA16F }, // albedo(RGB), occlusion(A)
				{ ImageFormat::RGBA16F }, // metalness(R), roughness(A)
				{ ImageFormat::RGBA16F }, // (world)normal(RGB), unused(A) (why RGB10_A2 clear failed?)
				{ ImageFormat::DEPTH24_STENCIL8 } };
			FBOInfo.Samples = 1;
			Shared<FrameBuffer> FBO = FrameBuffer::Create(FBOInfo);
			
			RenderPipelineInfo pipelineInfo;
			pipelineInfo.Layout = {
				{ ShaderDataType::Float3, "a_Position" },
				{ ShaderDataType::Float3, "a_Normal" },
				{ ShaderDataType::Float3, "a_Tangent" },
				{ ShaderDataType::Float3, "a_Binormal" },
				{ ShaderDataType::Float2, "a_TexCoord" },
			};
			pipelineInfo.Shader = LUMEN_ASSET_MANAGER->GetRuntimeAsset<Shader>("PBRStaticShader");
			
			RenderPassInfo renderPassInfo;
			renderPassInfo.Target = FBO;
			pipelineInfo.RenderPass = RenderPass::Create(renderPassInfo);
			s_Data.GeometryPipeline = RenderPipeline::Create(pipelineInfo);
		}

		{
			FrameBufferInfo FBOInfo;
			FBOInfo.Width = 1920;
			FBOInfo.Height = 1080;
			FBOInfo.Attachments = { { ImageFormat::RGBA16F } };
			FBOInfo.Samples = 1;
			Shared<FrameBuffer> FBO = FrameBuffer::Create(FBOInfo);

			RenderPipelineInfo pipelineInfo;
			pipelineInfo.Layout = {
				{ ShaderDataType::Float3, "a_Position" },
				{ ShaderDataType::Float2, "a_TexCoord" }
			};
			pipelineInfo.Shader = LUMEN_ASSET_MANAGER->GetRuntimeAsset<Shader>("LightingShader");

			RenderPassInfo renderPassInfo;
			renderPassInfo.Target = FBO;
			pipelineInfo.RenderPass = RenderPass::Create(renderPassInfo);
			s_Data.LightingPipeline = RenderPipeline::Create(pipelineInfo);
		}

		{
			FrameBufferInfo FBOInfo;
			FBOInfo.Width = 1920;
			FBOInfo.Height = 1080;
			FBOInfo.Attachments = { { ImageFormat::RGBA16F } };
			FBOInfo.Samples = 1;
			Shared<FrameBuffer> FBO = FrameBuffer::Create(FBOInfo);

			RenderPipelineInfo pipelineInfo;
			pipelineInfo.Layout = {
				{ ShaderDataType::Float3, "a_Position" },
				{ ShaderDataType::Float2, "a_TexCoord" }
			};
			pipelineInfo.Shader = LUMEN_ASSET_MANAGER->GetRuntimeAsset<Shader>("SkyboxShader");

			RenderPassInfo renderPassInfo;
			renderPassInfo.Target = FBO;
			pipelineInfo.RenderPass = RenderPass::Create(renderPassInfo);
			s_Data.SkyboxPipeline = RenderPipeline::Create(pipelineInfo);
		}

		{
			FrameBufferInfo FBOInfo;
			FBOInfo.Width = 1920;
			FBOInfo.Height = 1080;
			FBOInfo.Attachments = { { ImageFormat::RGBA16F } };
			FBOInfo.Samples = 1;
			Shared<FrameBuffer> FBO = FrameBuffer::Create(FBOInfo);

			RenderPipelineInfo pipelineInfo;
			pipelineInfo.Layout = {
				{ ShaderDataType::Float3, "a_Position" },
				{ ShaderDataType::Float2, "a_TexCoord" }
			};
			pipelineInfo.Shader = LUMEN_ASSET_MANAGER->GetRuntimeAsset<Shader>("BloomShader");

			RenderPassInfo renderPassInfo;
			renderPassInfo.Target = FBO;
			pipelineInfo.RenderPass = RenderPass::Create(renderPassInfo);
			s_Data.BloomPipeline = RenderPipeline::Create(pipelineInfo);
		}

		{
			FrameBufferInfo FBOInfo;
			FBOInfo.Width = 1920;
			FBOInfo.Height = 1080;
			FBOInfo.Attachments = { { ImageFormat::RGBA16F }, { ImageFormat::RGBA16F } };
			FBOInfo.Samples = 1;
			Shared<FrameBuffer> FBO = FrameBuffer::Create(FBOInfo);

			RenderPipelineInfo pipelineInfo;
			pipelineInfo.Layout = {
				{ ShaderDataType::Float3, "a_Position" },
				{ ShaderDataType::Float2, "a_TexCoord" }
			};
			pipelineInfo.Shader = LUMEN_ASSET_MANAGER->GetRuntimeAsset<Shader>("GaussianBlurShader");

			RenderPassInfo renderPassInfo;
			renderPassInfo.Target = FBO;
			pipelineInfo.RenderPass = RenderPass::Create(renderPassInfo);
			s_Data.GaussianBlurPipeline = RenderPipeline::Create(pipelineInfo);
		}

		{
			FrameBufferInfo FBOInfo;
			FBOInfo.Width = 1920;
			FBOInfo.Height = 1080;
			FBOInfo.Attachments = { { ImageFormat::RGBA8 } };
			FBOInfo.Samples = 1;
			Shared<FrameBuffer> FBO = FrameBuffer::Create(FBOInfo);
	
			RenderPipelineInfo pipelineInfo;
			pipelineInfo.Layout = { 
				{ ShaderDataType::Float3, "a_Position" },
				{ ShaderDataType::Float2, "a_TexCoord" }
			};
			pipelineInfo.Shader = LUMEN_ASSET_MANAGER->GetRuntimeAsset<Shader>("CompositeShader");
	
			RenderPassInfo renderPassInfo;
			renderPassInfo.Target = FBO;
			pipelineInfo.RenderPass = RenderPass::Create(renderPassInfo);
			s_Data.CompositePipeline = RenderPipeline::Create(pipelineInfo);
		}
	}

	void RenderSystem::ShutDown()
	{
		m_RHI.reset();
	}

	void RenderSystem::Begin()
	{
	}

	void RenderSystem::Update()
	{
		SetLights();

		//TestPass();

		//ShadowPass();
		GeometryPass();
		LightingPass();
		SkyboxPass();
		PostProcessPass();
		CompositePass();

		//SpritePass();
	}

	void RenderSystem::End()
	{
		m_RHI->SwapBuffers();
	}

	void RenderSystem::OnSetViewport(uint32_t width, uint32_t height)
	{
		if (s_Data.ViewportWidth != width || s_Data.ViewportHeight != height)
		{
			s_Data.ViewportWidth = width;
			s_Data.ViewportHeight = height;

			s_Data.GeometryPipeline->GetInfo().RenderPass->GetInfo().Target->Resize(width, height);
			s_Data.LightingPipeline->GetInfo().RenderPass->GetInfo().Target->Resize(width, height);
			s_Data.SkyboxPipeline->GetInfo().RenderPass->GetInfo().Target->Resize(width, height);
			s_Data.GaussianBlurPipeline->GetInfo().RenderPass->GetInfo().Target->Resize(width, height);
			s_Data.CompositePipeline->GetInfo().RenderPass->GetInfo().Target->Resize(width, height);
		}
	}

	void RenderSystem::OnSetViewProject(const glm::mat4& view, const glm::mat4& project, const glm::vec3& viewPosition)
	{
		s_Data.View = view;
		s_Data.Projection = project;
		s_Data.viewPosition = viewPosition;

		/*s_Data.VPUBO->SetData(glm::value_ptr(s_Data.View), 64, 0);
		s_Data.VPUBO->SetData(glm::value_ptr(s_Data.Projection), 64, 64);*/
	}

	void RenderSystem::SetSkybox(AssetHandle handle)
	{
		s_LightingData.Skybox = handle;
	}

	void RenderSystem::SubmitLight(const LightComponent& light)
	{
		s_Lights.emplace_back(light);
	}

	void RenderSystem::SubmitMesh(const glm::mat4& transform, const MeshRendererComponent& mesh)
	{
		meshDrawList.emplace_back(transform, mesh);
	}

	void RenderSystem::SubmitSprite(const glm::mat4& transform, const SpriteRendererComponent& sprite)
	{
		spriteDrawList.emplace_back(transform, sprite);
	}

	void RenderSystem::SetLights()
	{
		glm::uvec4 lightNum(0);
		uint32_t offset = 0;
		for (auto& light : s_Lights)
		{
			if (light.Type == LightComponent::LightType::Directional)
			{
				if (lightNum[0] >= 5)
					continue;
				offset = 0 + lightNum[0] * 96;
				lightNum[0]++;

				s_Data.LightingPipeline->GetInfo().Shader->SetUniformBuffer("Lights", glm::value_ptr(light.Color), 12, offset);
				s_Data.LightingPipeline->GetInfo().Shader->SetUniformBuffer("Lights", &light.Intensity, 4, offset + 12);
				s_Data.LightingPipeline->GetInfo().Shader->SetUniformBuffer("Lights", glm::value_ptr(light.Direction), 12, offset + 16);

				float near = 1.f, far = 10.f;
				glm::mat4 lightProject = glm::ortho(-10.f, 10.f, -10.f, 10.f, near, far);
				glm::vec3 position = glm::vec3(0.f, 5.f, 5.f);
				glm::mat4 lightView = glm::lookAt(position, position + s_Lights[0].Direction, glm::vec3(0.f, 1.f, 0.f));
				s_Data.LightingPipeline->GetInfo().Shader->SetUniformBuffer("Lights", glm::value_ptr(lightProject * lightView), 64, offset + 32);
			}
			else if (light.Type == LightComponent::LightType::Spot)
			{
				if (lightNum[1] >= 5)
					continue;
				offset = 96 * 5 + lightNum[1] * 112;
				lightNum[1]++;

				float cutoff = glm::cos(glm::radians(light.Cutoff));
				float outerCutoff = glm::cos(glm::radians(light.OuterCutoff));

				s_Data.LightingPipeline->GetInfo().Shader->SetUniformBuffer("Lights", glm::value_ptr(light.Color), 12, offset);
				s_Data.LightingPipeline->GetInfo().Shader->SetUniformBuffer("Lights", &light.Intensity, 4, offset + 12);
				s_Data.LightingPipeline->GetInfo().Shader->SetUniformBuffer("Lights", glm::value_ptr(light.Position), 12, offset + 16);
				s_Data.LightingPipeline->GetInfo().Shader->SetUniformBuffer("Lights", &cutoff, 4, offset + 28);
				s_Data.LightingPipeline->GetInfo().Shader->SetUniformBuffer("Lights", glm::value_ptr(light.Direction), 12, offset + 32);
				s_Data.LightingPipeline->GetInfo().Shader->SetUniformBuffer("Lights", &outerCutoff, 4, offset + 44);

				float near = 0.01f, far = 5.f;
				glm::mat4 lightProject = glm::perspective(light.OuterCutoff * 2.f, 1.f, near, far);
				glm::mat4 lightView = glm::lookAt(light.Position, light.Position + light.Direction, glm::vec3(0.f, 1.f, 0.f));
				s_Data.LightingPipeline->GetInfo().Shader->SetUniformBuffer("Lights", glm::value_ptr(lightProject * lightView), 64, offset + 48);
			}
			else if (light.Type == LightComponent::LightType::Point)
			{
				if (lightNum[2] >= 5)
					continue;
				offset = 96 * 5 + 112 * 5 + lightNum[2] * 32;
				lightNum[2]++;

				s_Data.LightingPipeline->GetInfo().Shader->SetUniformBuffer("Lights", glm::value_ptr(light.Color), 12, offset);
				s_Data.LightingPipeline->GetInfo().Shader->SetUniformBuffer("Lights", &light.Intensity, 4, offset + 12);
				s_Data.LightingPipeline->GetInfo().Shader->SetUniformBuffer("Lights", glm::value_ptr(light.Position), 12, offset + 16);

				float near = 0.1f, far = 5.f;
				s_Data.LightingPipeline->GetInfo().Shader->SetUniformBuffer("Lights", &far, 4, offset + 28);
			}
		}
		offset = 96 * 5 + 112 * 5 + 32 * 5;
		s_Data.LightingPipeline->GetInfo().Shader->SetUniformBuffer("Lights", glm::value_ptr(lightNum), 16, offset);
		uint32_t useEnvironmentMap = s_LightingData.Skybox != 0;
		s_Data.LightingPipeline->GetInfo().Shader->SetUniformBuffer("Lights", &useEnvironmentMap, 4, offset + 16);

		s_Lights.clear();
	}

	void RenderSystem::TestPass()
	{
		constexpr float clearColor[4] = { 0.f, 0.f, 0.f, 1.f };
		s_Data.TestPipeline->GetInfo().RenderPass->GetInfo().Target->ClearColor(0, clearColor);
		s_Data.TestPipeline->GetInfo().RenderPass->GetInfo().Target->ClearDepthStencil(1.0f, 0);
		s_Data.TestPipeline->GetInfo().RenderPass->GetInfo().Target->Bind();

		for (auto& [transform, mesh] : meshDrawList)
			m_RHI->RenderMesh(s_Data.TestPipeline, transform, mesh.Mesh, mesh.Materials);

		meshDrawList.clear();
		s_Data.TestPipeline->GetInfo().RenderPass->GetInfo().Target->Unbind();
	}

	void RenderSystem::ShadowPass()
	{
		// directional
		/*{
			constexpr float clearDepth = 1.f;
			s_Data.DirectionalShadowPipeline->GetInfo().RenderPass->GetInfo().Target->ClearDepth(&clearDepth);
			s_Data.DirectionalShadowPipeline->GetInfo().RenderPass->GetInfo().Target->Bind();

			for (auto& [transform, mesh] : meshDrawList)
				m_RHI->RenderShadow(s_Data.DirectionalShadowPipeline, transform, mesh.Mesh);

			s_Data.DirectionalShadowPipeline->GetInfo().RenderPass->GetInfo().Target->Unbind();
		}

		{
			constexpr float clearDepth = 1.f;
			s_Data.SpotShadowPipeline->GetInfo().RenderPass->GetInfo().Target->ClearDepth(&clearDepth);
			s_Data.SpotShadowPipeline->GetInfo().RenderPass->GetInfo().Target->Bind();

			for (auto& [transform, mesh] : meshDrawList)
				m_RHI->RenderShadow(s_Data.SpotShadowPipeline, transform, mesh.Mesh);

			s_Data.SpotShadowPipeline->GetInfo().RenderPass->GetInfo().Target->Unbind();
		}*/

		glm::uvec4 lightNum(0);
		for (auto& light : s_Lights)
		{
			if (light.Type == LightComponent::LightType::Directional)
			{
				if (lightNum[0] >= 5)
					continue;
				lightNum[0]++;


			}
			else if (light.Type == LightComponent::LightType::Spot)
			{
				if (lightNum[1] >= 5)
					continue;
				lightNum[1]++;


			}
			else if (light.Type == LightComponent::LightType::Point)
			{
				if (lightNum[2] >= 5)
					continue;
				lightNum[2]++;

				float near = 0.1f, far = 5.f;
				glm::mat4 shadowProj = glm::perspective(glm::radians(90.0f), 1.f, near, far);
				std::array<glm::mat4, 6> shadowTransforms = {
					shadowProj * glm::lookAt(light.Position, light.Position + glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
					shadowProj * glm::lookAt(light.Position, light.Position + glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
					shadowProj * glm::lookAt(light.Position, light.Position + glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
					shadowProj * glm::lookAt(light.Position, light.Position + glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)),
					shadowProj * glm::lookAt(light.Position, light.Position + glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
					shadowProj * glm::lookAt(light.Position, light.Position + glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f))
				};

				constexpr float clearDepth = 1.f;
				s_Data.PointShadowPipeline->GetInfo().RenderPass->GetInfo().Target->ClearDepth(&clearDepth);
				s_Data.PointShadowPipeline->GetInfo().RenderPass->GetInfo().Target->Bind();

				s_Data.PointShadowPipeline->GetInfo().Shader->SetUniformBuffer("View", shadowTransforms.data(),  64 * 6, 0);
				s_Data.PointShadowPipeline->GetInfo().Shader->SetUniformBuffer("View", &lightNum[2], 4, 64 * 6);

				for (auto& [transform, mesh] : meshDrawList)
					m_RHI->RenderMesh(s_Data.PointShadowPipeline, transform, mesh.Mesh, mesh.Materials);

				s_Data.PointShadowPipeline->GetInfo().RenderPass->GetInfo().Target->Unbind();
			}
		}
	}

	void RenderSystem::GeometryPass()
	{
		constexpr float clearColor[4] = { 0.f, 0.f, 0.f, 0.f };
		s_Data.GeometryPipeline->GetInfo().RenderPass->GetInfo().Target->ClearColor(0, clearColor);
		s_Data.GeometryPipeline->GetInfo().RenderPass->GetInfo().Target->ClearColor(1, clearColor);
		s_Data.GeometryPipeline->GetInfo().RenderPass->GetInfo().Target->ClearColor(2, clearColor);
		s_Data.GeometryPipeline->GetInfo().RenderPass->GetInfo().Target->ClearColor(3, clearColor);
		s_Data.GeometryPipeline->GetInfo().RenderPass->GetInfo().Target->ClearDepthStencil(1.0f, 0);
		s_Data.GeometryPipeline->GetInfo().RenderPass->GetInfo().Target->Bind();

		s_Data.GeometryPipeline->GetInfo().Shader->SetUniformBuffer("View", glm::value_ptr(s_Data.View), 64, 0);
		s_Data.GeometryPipeline->GetInfo().Shader->SetUniformBuffer("View", glm::value_ptr(s_Data.Projection), 64, 64);
		s_Data.GeometryPipeline->GetInfo().Shader->SetUniformBuffer("View", glm::value_ptr(s_Data.viewPosition), 12, 128);

		for (auto& [transform, mesh] : meshDrawList)
			m_RHI->RenderMesh(s_Data.GeometryPipeline, transform, mesh.Mesh, mesh.Materials);

		meshDrawList.clear();
		s_Data.GeometryPipeline->GetInfo().RenderPass->GetInfo().Target->Unbind();
	}

	void RenderSystem::LightingPass()
	{
		if (!s_LightingData.Skybox)
		{
			s_LightingData.SkyboxIrradiance = nullptr;
			s_LightingData.SkyboxPrefiltered = nullptr;
		}
		else
		{
			auto mat = LUMEN_ASSET_MANAGER->GetAsset<Material>(s_LightingData.Skybox);
			if (mat->GetModified())
			{
				auto [irradiance, prefiltered] = m_RHI->PreprocessSkybox(s_LightingData.Skybox);
				s_LightingData.SkyboxIrradiance = irradiance;
				s_LightingData.SkyboxPrefiltered = prefiltered;

				mat->SetModified(false);
			}
		}

		constexpr float clearColor[4] = { 0.f, 0.f, 0.f, 1.f };
		s_Data.LightingPipeline->GetInfo().RenderPass->GetInfo().Target->ClearColor(0, clearColor);
		s_Data.LightingPipeline->GetInfo().RenderPass->GetInfo().Target->Bind();

		auto tex = s_Data.GeometryPipeline->GetInfo().RenderPass->GetInfo().Target->GetColorAttachmentRendererID(0);
		m_RHI->BindTexture(2, tex);
		tex = s_Data.GeometryPipeline->GetInfo().RenderPass->GetInfo().Target->GetColorAttachmentRendererID(1);
		m_RHI->BindTexture(3, tex);
		tex = s_Data.GeometryPipeline->GetInfo().RenderPass->GetInfo().Target->GetColorAttachmentRendererID(2);
		m_RHI->BindTexture(4, tex);
		tex = s_Data.GeometryPipeline->GetInfo().RenderPass->GetInfo().Target->GetColorAttachmentRendererID(3);
		m_RHI->BindTexture(5, tex);
		if (s_LightingData.Skybox && s_LightingData.SkyboxIrradiance && s_LightingData.SkyboxPrefiltered)
		{
			s_LightingData.SkyboxIrradiance->Bind(6);
			s_LightingData.SkyboxPrefiltered->Bind(7);
			LUMEN_ASSET_MANAGER->GetRuntimeAsset<Texture>("BRDFLUTTexture")->Bind(8);
		}

		m_RHI->RenderLighting(s_Data.LightingPipeline, s_Data.QuadVBO, s_Data.QuadIBO);

		s_Data.LightingPipeline->GetInfo().RenderPass->GetInfo().Target->Unbind();
	}

	void RenderSystem::SkyboxPass()
	{
		if (!s_LightingData.Skybox)
			return;

		constexpr float clearColor[4] = { 0., 0.f, 0.f, 1.f };
		s_Data.SkyboxPipeline->GetInfo().RenderPass->GetInfo().Target->ClearColor(0, clearColor);
		s_Data.SkyboxPipeline->GetInfo().RenderPass->GetInfo().Target->Bind();

		auto tex = s_Data.GeometryPipeline->GetInfo().RenderPass->GetInfo().Target->GetDepthAttachmentRendererID();
		m_RHI->BindTexture(1, tex);
		tex = s_Data.LightingPipeline->GetInfo().RenderPass->GetInfo().Target->GetColorAttachmentRendererID();
		m_RHI->BindTexture(2, tex);
		s_Data.SkyboxPipeline->GetInfo().Shader->Bind();

		auto material = LUMEN_ASSET_MANAGER->GetAsset<Material>(s_LightingData.Skybox);
		material->Bind();

		m_RHI->RenderSkybox(s_Data.SkyboxPipeline, s_Data.QuadVBO, s_Data.QuadIBO, s_LightingData.Skybox);

		s_Data.SkyboxPipeline->GetInfo().RenderPass->GetInfo().Target->Unbind();
	}

	void RenderSystem::PostProcessPass()
	{
		constexpr float clearColor[4] = { 0.f, 0.f, 0.f, 1.f };
		s_Data.BloomPipeline->GetInfo().RenderPass->GetInfo().Target->ClearColor(0, clearColor);
		s_Data.BloomPipeline->GetInfo().RenderPass->GetInfo().Target->Bind();

		uint32_t tex = 0;
		if (s_LightingData.Skybox)
			tex = s_Data.SkyboxPipeline->GetInfo().RenderPass->GetInfo().Target->GetColorAttachmentRendererID(0);
		else
			tex = s_Data.LightingPipeline->GetInfo().RenderPass->GetInfo().Target->GetColorAttachmentRendererID(0);
		m_RHI->BindTexture(0, tex);

		m_RHI->RenderBloom(s_Data.BloomPipeline, s_Data.QuadVBO, s_Data.QuadIBO);

		s_Data.BloomPipeline->GetInfo().RenderPass->GetInfo().Target->Unbind();

		s_Data.GaussianBlurPipeline->GetInfo().RenderPass->GetInfo().Target->ClearColor(0, clearColor);
		s_Data.GaussianBlurPipeline->GetInfo().RenderPass->GetInfo().Target->ClearColor(1, clearColor);
		s_Data.GaussianBlurPipeline->GetInfo().RenderPass->GetInfo().Target->Bind();

		tex = s_Data.BloomPipeline->GetInfo().RenderPass->GetInfo().Target->GetColorAttachmentRendererID();
		m_RHI->BindTexture(0, tex);

		m_RHI->RenderGaussianBlur(s_Data.GaussianBlurPipeline, s_Data.QuadVBO, s_Data.QuadIBO);

		s_Data.GaussianBlurPipeline->GetInfo().RenderPass->GetInfo().Target->Unbind();
	}

	void RenderSystem::CompositePass()
	{
		constexpr float clearColor[4] = { 0.f, 0.f, 0.f, 1.f };
		s_Data.CompositePipeline->GetInfo().RenderPass->GetInfo().Target->ClearColor(0, clearColor);
		s_Data.CompositePipeline->GetInfo().RenderPass->GetInfo().Target->Bind();

		uint32_t tex = 0;
		if (s_LightingData.Skybox)
			tex = s_Data.SkyboxPipeline->GetInfo().RenderPass->GetInfo().Target->GetColorAttachmentRendererID(0);
		else
			tex = s_Data.LightingPipeline->GetInfo().RenderPass->GetInfo().Target->GetColorAttachmentRendererID(0);
		m_RHI->BindTexture(0, tex);
		tex = s_Data.GaussianBlurPipeline->GetInfo().RenderPass->GetInfo().Target->GetColorAttachmentRendererID(0);
		m_RHI->BindTexture(1, tex);

		m_RHI->RenderComposite(s_Data.CompositePipeline, s_Data.QuadVBO, s_Data.QuadIBO);

		s_Data.CompositePipeline->GetInfo().RenderPass->GetInfo().Target->Unbind();
	}

	void RenderSystem::SpritePass()
	{
		constexpr float clearColor[4] = { 0.f, 0.f, 0.f, 1.f };
		s_Data.SpritePipeline->GetInfo().RenderPass->GetInfo().Target->ClearColor(0, clearColor);
		s_Data.SpritePipeline->GetInfo().RenderPass->GetInfo().Target->Bind();

		for (auto& [transform, sprite] : spriteDrawList)
		{
			if (s_SpriteBatch.IndexCount >= SpriteBatch::MaxIndices)
			{
				m_RHI->RenderSprites(s_Data.SpritePipeline, s_SpriteBatch);
				s_SpriteBatch.Vertices.clear();
				s_SpriteBatch.IndexCount = 0;
				s_SpriteBatch.textureSlotIndex = 1;
			}

			constexpr size_t spriteVertexCount = 4;
			constexpr glm::vec4 vertexPositions[4] ={
				{ -0.5f, -0.5f, 0.f, 1.f },
				{ 0.5f, -0.5f, 0.f, 1.f },
				{ 0.5f,  0.5f, 0.f, 1.f },
				{ -0.5f,  0.5f, 0.f, 1.f }
			};
			constexpr glm::vec2 textureCoords[] = { {0.f, 0.f}, {1.f, 0.f}, {1.f, 1.f}, {0.f, 1.f} };
			float textureIndex = 0.f;

			if (sprite.texture)
			{
				auto texture = LUMEN_ASSET_MANAGER->GetAsset<Texture>(sprite.texture);

				for (uint32_t i = 1; i < s_SpriteBatch.textureSlotIndex; i++)
				{
					if (s_SpriteBatch.textureSlots[i]->GetRendererID() == texture->GetRendererID())
					{
						textureIndex = (float)i;
						break;
					}
				}

				if (textureIndex == 0.f)
				{
					textureIndex = (float)s_SpriteBatch.textureSlotIndex;
					s_SpriteBatch.textureSlots[s_SpriteBatch.textureSlotIndex] = 
						LUMEN_ASSET_MANAGER->GetAsset<Texture>(sprite.texture);
					s_SpriteBatch.textureSlotIndex++;
				}
			}

			for (size_t i = 0; i < spriteVertexCount; ++i)
			{
				auto& vertex = s_SpriteBatch.Vertices.emplace_back();
				vertex.position = transform * vertexPositions[i];
				vertex.color = sprite.color;
				vertex.texCoord = textureCoords[i];
				vertex.texIndex = textureIndex;
				vertex.tilingFactor = sprite.tilingFactor;
			}

			s_SpriteBatch.IndexCount += 6;
		}

		m_RHI->RenderSprites(s_Data.SpritePipeline, s_SpriteBatch);
		s_SpriteBatch.Vertices.clear();
		s_SpriteBatch.IndexCount = 0;
		s_SpriteBatch.textureSlotIndex = 1;

		spriteDrawList.clear();

		s_Data.SpritePipeline->GetInfo().RenderPass->GetInfo().Target->Unbind();
	}

	uint32_t RenderSystem::GetFinalImageRendererID()
	{
		return s_Data.CompositePipeline->GetInfo().RenderPass->GetInfo().Target->GetColorAttachmentRendererID();
	}
}