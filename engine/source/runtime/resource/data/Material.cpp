#include "runtime/resource/data/Material.h"

#include "runtime/function/global/GlobalContext.h"
#include "runtime/function/render/RenderSystem.h"
#include "runtime/resource/asset/AssetManager.h"
#include "runtime/core/Macro.h"

namespace Lumen
{
	Material::Material(const MaterialInfo& info)
		: m_Info(info)
	{
		auto shader = LUMEN_ASSET_MANAGER->GetRuntimeAsset<Shader>(m_Info.Shader);
		const auto& mat = shader->GetPushConstantBuffers().at("u_Material");
		m_UniformStorageBuffer.resize(mat.Size, 0);
	}

	void Material::SetShader(const std::string& handle)
	{
		m_Info.Shader = handle;
		auto shader = LUMEN_ASSET_MANAGER->GetRuntimeAsset<Shader>(m_Info.Shader);
		const auto& mat = shader->GetPushConstantBuffers().at("u_Material");
		m_UniformStorageBuffer.clear();
		m_UniformStorageBuffer.resize(mat.Size, 0);
	}

	void Material::Set(const std::string& name, bool value)
	{
		Set<bool>(name, value);
	}

	void Material::Set(const std::string& name, int32_t value)
	{
		Set<int32_t>(name, value);
	}
	
	void Material::Set(const std::string& name, uint32_t value)
	{
		Set<uint32_t>(name, value);
	}
	
	void Material::Set(const std::string& name, float value)
	{
		Set<float>(name, value);
	}
	
	void Material::Set(const std::string& name, const glm::vec2& value)
	{
		Set<glm::vec2>(name, value);
	}
	
	void Material::Set(const std::string& name, const glm::vec3& value)
	{
		Set<glm::vec3>(name, value);
	}
	
	void Material::Set(const std::string& name, const glm::vec4& value)
	{
		Set<glm::vec4>(name, value);
	}
	
	void Material::Set(const std::string& name, const glm::mat2& value)
	{
		Set<glm::mat2>(name, value);
	}
	
	void Material::Set(const std::string& name, const glm::mat3& value)
	{
		Set<glm::mat3>(name, value);
	}
	
	void Material::Set(const std::string& name, const glm::mat4& value)
	{
		Set<glm::mat4>(name, value);
	}

	void Material::Set(const std::string& name, AssetHandle texture)
	{
		const auto& decl = FindResourceDeclaration(name);
		uint32_t slot = decl.GetRegister();
		m_Textures[slot] = texture;
	}

	bool& Material::GetBool(const std::string& name)
	{
		return Get<bool>(name);
	}

	int32_t& Material::GetInt(const std::string& name)
	{
		return Get<int32_t>(name);
	}

	uint32_t& Material::GetUInt(const std::string& name)
	{
		return Get<uint32_t>(name);
	}

	float& Material::GetFloat(const std::string& name)
	{
		return Get<float>(name);
	}

	glm::vec2& Material::GetVector2(const std::string& name)
	{
		return Get<glm::vec2>(name);
	}

	glm::vec3& Material::GetVector3(const std::string& name)
	{
		return Get<glm::vec3>(name);
	}

	glm::vec4& Material::GetVector4(const std::string& name)
	{
		return Get<glm::vec4>(name);
	}

	glm::mat2& Material::GetMatrix2(const std::string& name)
	{
		return Get<glm::mat2>(name);
	}

	glm::mat3& Material::GetMatrix3(const std::string& name)
	{
		return Get<glm::mat3>(name);
	}

	glm::mat4& Material::GetMatrix4(const std::string& name)
	{
		return Get<glm::mat4>(name);
	}

	AssetHandle& Material::GetTexture(const std::string& name)
	{
		const auto& decl = FindResourceDeclaration(name);
		uint32_t slot = decl.GetRegister();
		return m_Textures.at(slot);
	}

	const ShaderUniform& Material::FindUniformDeclaration(const std::string& name) const
	{
		auto shader = LUMEN_ASSET_MANAGER->GetRuntimeAsset<Shader>(m_Info.Shader);
		const auto& mat = shader->GetPushConstantBuffers().at("u_Material");
		return mat.Uniforms.at(name);
	}

	const ShaderResourceDeclaration& Material::FindResourceDeclaration(const std::string& name) const
	{
		auto shader = LUMEN_ASSET_MANAGER->GetRuntimeAsset<Shader>(m_Info.Shader);
		return shader->GetResourceDeclarations().at(name);
	}

	void Material::Write(const void* data, uint32_t size, uint32_t offset)
	{
		ASSERT(data);
		ASSERT(offset + size <= m_UniformStorageBuffer.size());
		
		memcpy(m_UniformStorageBuffer.data() + offset, data, size);
	}

	void* Material::Read(uint32_t offset)
	{
		return m_UniformStorageBuffer.data() + offset;
	}

	void Material::Bind()
	{
		auto shader = LUMEN_ASSET_MANAGER->GetRuntimeAsset<Shader>(m_Info.Shader);
		shader->Bind();
		const auto& buffer = shader->GetPushConstantBuffers().at("u_Material");

		for (auto& [name, uniform] : buffer.Uniforms)
		{
			switch (uniform.GetType())
			{
			case ShaderUniformType::Bool:
			{
				bool value = *reinterpret_cast<bool*>(Read(uniform.GetOffset()));
				shader->SetUniform(name, value);
				break;
			}
			case ShaderUniformType::Int:
			{
				int32_t value = *reinterpret_cast<int32_t*>(Read(uniform.GetOffset()));
				shader->SetUniform(name, value);
				break;
			}
			case ShaderUniformType::UInt:
			{
				uint32_t value = *reinterpret_cast<uint32_t*>(Read(uniform.GetOffset()));
				shader->SetUniform(name, value);
				break;
			}
			case ShaderUniformType::Float:
			{
				float value = *reinterpret_cast<float*>(Read(uniform.GetOffset()));
				shader->SetUniform(name, value);
				break;
			}
			case ShaderUniformType::Vec2:
			{
				const glm::vec2& value = *reinterpret_cast<glm::vec2*>(Read(uniform.GetOffset()));
				shader->SetUniform(name, value);
				break;
			}
			case ShaderUniformType::Vec3:
			{
				const glm::vec3& value = *reinterpret_cast<glm::vec3*>(Read(uniform.GetOffset()));
				shader->SetUniform(name, value);
				break;
			}
			case ShaderUniformType::Vec4:
			{
				const glm::vec4& value = *reinterpret_cast<glm::vec4*>(Read(uniform.GetOffset()));
				shader->SetUniform(name, value);
				break;
			}
			case ShaderUniformType::Mat2:
			{
				const glm::mat2& value = *reinterpret_cast<glm::mat2*>(Read(uniform.GetOffset()));
				shader->SetUniform(name, value);
				break;
			}
			case ShaderUniformType::Mat3:
			{
				const glm::mat3& value = *reinterpret_cast<glm::mat3*>(Read(uniform.GetOffset()));
				shader->SetUniform(name, value);
				break;
			}
			case ShaderUniformType::Mat4:
			{
				const glm::mat4& value = *reinterpret_cast<glm::mat4*>(Read(uniform.GetOffset()));
				shader->SetUniform(name, value);
				break;
			}
			default:
			{
				ASSERT(false);
				break;
			}
			}
		}

		for (auto& [slot, texture] : m_Textures)
		{
			if (texture)
			{
				auto tex = LUMEN_ASSET_MANAGER->GetAsset<Texture>(texture);
				tex->Bind(slot);
			}
		}
	}
}