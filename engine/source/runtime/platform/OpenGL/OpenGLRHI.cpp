#include "runtime/platform/OpenGL/OpenGLRHI.h"

#include "runtime/function/global/GlobalContext.h"
#include "runtime/function/render/Window.h"
#include "runtime/function/render/RenderPipeline.h"
#include "runtime/function/render/SpriteBatch.h"
#include "runtime/resource/asset/AssetManager.h"
#include "runtime/resource/data/Material.h"
#include "runtime/resource/data/Mesh.h"
#include "runtime/core/Log.h"
#include "runtime/core/Macro.h"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glad/glad.h>

#include <cassert>

namespace Lumen
{
	static void OpenGLMessageCallback(
		unsigned source,
		unsigned type,
		unsigned id,
		unsigned severity,
		int length,
		const char* message,
		const void* userParam)
	{
		switch (severity)
		{
		case GL_DEBUG_SEVERITY_HIGH:         LUMEN_RUNTIME_CRITICAL(message); break;
		case GL_DEBUG_SEVERITY_MEDIUM:       LUMEN_RUNTIME_ERROR(message); break;
		case GL_DEBUG_SEVERITY_LOW:          LUMEN_RUNTIME_WARN(message); break;
		case GL_DEBUG_SEVERITY_NOTIFICATION: LUMEN_RUNTIME_TRACE(message); break;
		default: ASSERT(false); break;
		}

		return;
	}

	OpenGLRHI::OpenGLRHI(const RHIInfo& info)
	{
		m_Window = info.window->GetWindow();

		// graphics context
		glfwMakeContextCurrent(m_Window);
		int status = gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
		ASSERT(status);

		LUMEN_RUNTIME_INFO("Graphics API: OpenGL");
		LUMEN_RUNTIME_INFO("  Vendor: {0}", (const char*)glGetString(GL_VENDOR));
		LUMEN_RUNTIME_INFO("  Renderer: {0}", (const char*)glGetString(GL_RENDERER));
		LUMEN_RUNTIME_INFO("  Version: {0}", (const char*)glGetString(GL_VERSION));

#ifdef LUMEN_DEBUG
		glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		glDebugMessageCallback(OpenGLMessageCallback, nullptr);

		glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION, 0, NULL, GL_FALSE);
#endif
	}

	OpenGLRHI::~OpenGLRHI()
	{
		m_Window = nullptr;
	}

	void OpenGLRHI::SetVSync(bool enabled)
	{
		// TODO: in-game option
		if (enabled)
			glfwSwapInterval(1);
		else
			glfwSwapInterval(0);
	}

	void OpenGLRHI::SetDepthTest(bool enable)
	{
		if (enable)
			glEnable(GL_DEPTH_TEST);
		else
			glDisable(GL_DEPTH_TEST);
	}

	void OpenGLRHI::SetBlend(bool enable)
	{
		if (enable)
		{
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		}
		else
		{
			glDisable(GL_BLEND);
		}
	}

	void OpenGLRHI::SwapBuffers()
	{
		glfwSwapBuffers(m_Window);
	}

	void OpenGLRHI::BindTexture(uint32_t slot, uint32_t texture)
	{
		glBindTextureUnit(slot, texture);
	}

	std::pair<Shared<Texture>, Shared<Texture>> OpenGLRHI::PreprocessSkybox(AssetHandle skybox)
	{
		// TODO: async
		ASSERT(skybox);

		auto mat = LUMEN_ASSET_MANAGER->GetAsset<Material>(skybox);

		if (auto& map = mat->GetTexture("u_SkyboxMap"))
		{
			auto skyboxMap = LUMEN_ASSET_MANAGER->GetAsset<Texture>(map);

			TextureInfo info;
			info.format = skyboxMap->GetFormat();
			info.shape = skyboxMap->GetShape();
			info.generateMipmaps = true;
			info.width = info.height = 32;
			auto irradianceMap = Texture::Create(info);

			auto irradianceShader = LUMEN_ASSET_MANAGER->GetRuntimeAsset<Shader>("SkyboxIrradianceShader");
			irradianceShader->Bind();
			skyboxMap->Bind(1);

			irradianceShader->SetUniform("u_Parameters.TintColor", mat->GetVector4("u_Material.TintColor"));
			irradianceShader->SetUniform("u_Parameters.Exposure", mat->GetFloat("u_Material.Exposure"));
			irradianceShader->SetUniform("u_Parameters.Rotation", mat->GetFloat("u_Material.Rotation"));

			glBindImageTexture(0, irradianceMap->GetRendererID(), 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA16F);

			glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

			glDispatchCompute(irradianceMap->GetWidth() / 32, irradianceMap->GetHeight() / 32, 6);
			glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT | GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
		
			glGenerateTextureMipmap(irradianceMap->GetRendererID());

			info.width = skyboxMap->GetWidth();
			info.height = skyboxMap->GetHeight();
			auto filteredMap = Texture::Create(info);

			/*glCopyImageSubData(skyboxMap->GetRendererID(), GL_TEXTURE_CUBE_MAP, 0, 0, 0, 0,
				filteredMap->GetRendererID(), GL_TEXTURE_CUBE_MAP, 0, 0, 0, 0,
				info.width, info.height, 6);*/

			auto prefilterShader = LUMEN_ASSET_MANAGER->GetRuntimeAsset<Shader>("SkyboxPrefilterShader");
			prefilterShader->Bind();
			skyboxMap->Bind(1);

			prefilterShader->SetUniform("u_Parameters.TintColor", mat->GetVector4("u_Material.TintColor"));
			prefilterShader->SetUniform("u_Parameters.Exposure", mat->GetFloat("u_Material.Exposure"));
			prefilterShader->SetUniform("u_Parameters.Rotation", mat->GetFloat("u_Material.Rotation"));

			const float deltaRoughness = 1.0f / std::max((float)(filteredMap->GetMipLevelCount() - 1.0f), 1.0f);
			for (uint32_t level = 0/*1*/, size = info.width/*/2*/; level < filteredMap->GetMipLevelCount(); level++, size /= 2)
			{
				glBindImageTexture(0, filteredMap->GetRendererID(), level, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA16F);

				prefilterShader->SetUniform("u_Parameters.Roughness", (float)level * deltaRoughness);
			
				const GLuint numGroups = std::max(1u, size / 32);
				glDispatchCompute(numGroups, numGroups, 6);
				glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT | GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
			}
		
			glDisable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

			return { irradianceMap, filteredMap };
		}

		return { nullptr, nullptr };
	}

	void OpenGLRHI::RenderShadow(Shared<RenderPipeline> pipeline, const glm::mat4& transform, AssetHandle mesh)
	{
		auto m = LUMEN_ASSET_MANAGER->GetAsset<Mesh>(mesh);
		m->GetVBO()->Bind();
		pipeline->Bind();
		m->GetIBO()->Bind();
		pipeline->GetInfo().Shader->Bind();
		pipeline->GetInfo().Shader->SetUniform("u_Model", transform);

		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
		glEnable(GL_DEPTH_TEST);

		for (size_t i = 0; i < m->GetSubmesh().size(); ++i)
		{
			auto& submesh = m->GetSubmesh()[i];

			glDrawElementsBaseVertex(GL_TRIANGLES, submesh.IndexCount, GL_UNSIGNED_INT, (void*)(sizeof(uint32_t) * submesh.StartIndex), submesh.StartVertex);
		}

		glDisable(GL_CULL_FACE);
		glDisable(GL_DEPTH_TEST);
	}

	void OpenGLRHI::RenderMesh(Shared<RenderPipeline> pipeline, const glm::mat4& transform, AssetHandle mesh, const std::vector<AssetHandle>& materials)
	{
		if (materials.empty() || !mesh)
			return;

		auto m = LUMEN_ASSET_MANAGER->GetAsset<Mesh>(mesh);
		m->GetVBO()->Bind();
		pipeline->Bind();
		m->GetIBO()->Bind();

		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
		glEnable(GL_DEPTH_TEST);

		for (size_t i = 0; i < m->GetSubmesh().size(); ++i)
		{
			if (!materials[i % materials.size()])
				continue;
			auto& submesh = m->GetSubmesh()[i];
			auto material = LUMEN_ASSET_MANAGER->GetAsset<Material>(materials[i % materials.size()]);
			if (material->GetShader() == "None")
				continue;
			material->Bind();
			//auto shader = LUMEN_ASSET_MANAGER->GetRuntimeAsset<Shader>(material->GetShader());
			pipeline->GetInfo().Shader->Bind();
			pipeline->GetInfo().Shader->SetUniform("u_Renderer.Transform", transform);
			
			/*if ((uint32_t)MaterialFlag::DepthTest & material->m_MaterialFlags)
				glEnable(GL_DEPTH_TEST);
			else
				glDisable(GL_DEPTH_TEST);*/

			glDrawElementsBaseVertex(GL_TRIANGLES, submesh.IndexCount, GL_UNSIGNED_INT, (void*)(sizeof(uint32_t) * submesh.StartIndex), submesh.StartVertex);
		}

		glDisable(GL_CULL_FACE);
		glDisable(GL_DEPTH_TEST);
	}

	void OpenGLRHI::RenderLighting(Shared<RenderPipeline> pipeline, Shared<VertexBuffer> VBO, Shared<IndexBuffer> IBO)
	{
		VBO->Bind();
		pipeline->Bind();
		IBO->Bind();
		pipeline->GetInfo().Shader->Bind();

		glDrawElements(GL_TRIANGLES, IBO->GetCount(), GL_UNSIGNED_INT, 0);
	}

	void OpenGLRHI::RenderSkybox(Shared<RenderPipeline> pipeline, Shared<VertexBuffer> VBO, Shared<IndexBuffer> IBO, AssetHandle skybox)
	{
		VBO->Bind();
		pipeline->Bind();
		IBO->Bind();

		glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

		glDrawElements(GL_TRIANGLES, IBO->GetCount(), GL_UNSIGNED_INT, 0);

		glDisable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
	}

	void OpenGLRHI::RenderSprites(Shared<RenderPipeline> pipeline, const SpriteBatch& batch)
	{
		batch.VBO->SetData(batch.Vertices.data(), static_cast<uint32_t>(batch.Vertices.size() * sizeof(SpriteVertex)));

		batch.VBO->Bind();
		pipeline->Bind();
		batch.IBO->Bind();
		pipeline->GetInfo().Shader->Bind();

		for (uint32_t i = 0; i < batch.textureSlotIndex; ++i)
			batch.textureSlots[i]->Bind(i);

		/*glDisable(GL_DEPTH_TEST);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);*/

		glDrawElements(GL_TRIANGLES, batch.IndexCount, GL_UNSIGNED_INT, batch.Indices.data());
	}

	void OpenGLRHI::RenderPostProcess(Shared<RenderPipeline> pipeline, Shared<VertexBuffer> VBO, Shared<IndexBuffer> IBO)
	{
		VBO->Bind();
		pipeline->Bind();
		IBO->Bind();
		pipeline->GetInfo().Shader->Bind();

		bool horizontal = true, firstIteration = true;
		uint32_t interationTime = 5;
		for (uint32_t i = 0; i < interationTime * 2; ++i)
		{
			pipeline->GetInfo().Shader->SetUniform("horizontal", horizontal);

			if (!firstIteration)
			{
				BindTexture(0, horizontal ?
					pipeline->GetInfo().RenderPass->GetInfo().Target->GetColorAttachmentRendererID(0) :
					pipeline->GetInfo().RenderPass->GetInfo().Target->GetColorAttachmentRendererID(1));
			}

			glDrawBuffer(horizontal ? GL_COLOR_ATTACHMENT1 : GL_COLOR_ATTACHMENT0);

			glDrawElements(GL_TRIANGLES, IBO->GetCount(), GL_UNSIGNED_INT, 0);

			if (firstIteration)
				firstIteration = false;

			horizontal = !horizontal;
		}
	}

	void OpenGLRHI::RenderBloom(Shared<RenderPipeline> pipeline, Shared<VertexBuffer> VBO, Shared<IndexBuffer> IBO)
	{
		VBO->Bind();
		pipeline->Bind();
		IBO->Bind();
		pipeline->GetInfo().Shader->Bind();

		glDrawElements(GL_TRIANGLES, IBO->GetCount(), GL_UNSIGNED_INT, 0);
	}

	void OpenGLRHI::RenderGaussianBlur(Shared<RenderPipeline> pipeline, Shared<VertexBuffer> VBO, Shared<IndexBuffer> IBO)
	{
		VBO->Bind();
		pipeline->Bind();
		IBO->Bind();
		pipeline->GetInfo().Shader->Bind();

		bool horizontal = true, firstIteration = true;
		uint32_t interationTime = 5;
		for (uint32_t i = 0; i < interationTime * 2; ++i)
		{
			pipeline->GetInfo().Shader->SetUniform("u_Parameters.Horizontal", horizontal);

			if (!firstIteration)
			{
				BindTexture(0, horizontal ?
					pipeline->GetInfo().RenderPass->GetInfo().Target->GetColorAttachmentRendererID(0) :
					pipeline->GetInfo().RenderPass->GetInfo().Target->GetColorAttachmentRendererID(1));
			}

			glDrawBuffer(horizontal ? GL_COLOR_ATTACHMENT1 : GL_COLOR_ATTACHMENT0);

			glDrawElements(GL_TRIANGLES, IBO->GetCount(), GL_UNSIGNED_INT, 0);

			if (firstIteration)
				firstIteration = false;

			horizontal = !horizontal;
		}
	}

	void OpenGLRHI::RenderComposite(Shared<RenderPipeline> pipeline, Shared<VertexBuffer> VBO, Shared<IndexBuffer> IBO)
	{
		VBO->Bind();
		pipeline->Bind();
		IBO->Bind();
		pipeline->GetInfo().Shader->Bind();
		pipeline->GetInfo().Shader->SetUniform("u_Parameters.HDR", true);
		pipeline->GetInfo().Shader->SetUniform("u_Parameters.Exposure", 1.f);

		glDrawElements(GL_TRIANGLES, IBO->GetCount(), GL_UNSIGNED_INT, 0);
	}
}