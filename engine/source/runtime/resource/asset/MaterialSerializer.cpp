#include "runtime/resource/asset/MaterialSerializer.h"

#include "runtime/function/global/GlobalContext.h"
#include "runtime/resource/asset/AssetManager.h"
#include "runtime/resource/asset/Serializer.h"
#include "runtime/core/Macro.h"

namespace Lumen
{
	Lumen::MaterialSerializer::MaterialSerializer(const Shared<Material>& material)
		: m_Material(material) { }

	bool MaterialSerializer::Serialize(const std::filesystem::path& path)
	{
		YAML::Emitter out;
		out << YAML::BeginMap;
		out << YAML::Key << "Material" << YAML::Value;
		{
			out << YAML::BeginMap;
			out << YAML::Key << "Shader" << YAML::Value << m_Material->GetShader(); // TODO: Use AssetHandle
			if (m_Material->GetShader() != "None")
				SerializeProperties(out);
			out << YAML::EndMap;
		}
		out << YAML::EndMap;

		std::ofstream fout(path);
		if (fout.is_open())
		{
			fout << out.c_str();
			fout.close();
			return true;
		}
		else
			return false;
	}

	void MaterialSerializer::SerializeProperties(YAML::Emitter& out)
	{
		auto shader = LUMEN_ASSET_MANAGER->GetRuntimeAsset<Shader>(m_Material->GetShader());
		const auto& buffer = shader->GetPushConstantBuffers().at("u_Material");

		for (auto& [name, uniform] : buffer.Uniforms)
		{
			switch (uniform.GetType())
			{
			case ShaderUniformType::Bool:
			{
				bool value = m_Material->GetBool(name);
				out << YAML::Key << name << YAML::Value << value;
				break;
			}
			case ShaderUniformType::Int:
			{
				int32_t value = m_Material->GetInt(name);
				out << YAML::Key << name << YAML::Value << value;
				break;
			}
			case ShaderUniformType::UInt:
			{
				uint32_t value = m_Material->GetUInt(name);
				out << YAML::Key << name << YAML::Value << value;
				break;
			}
			case ShaderUniformType::Float:
			{
				float value = m_Material->GetFloat(name);
				out << YAML::Key << name << YAML::Value << value;
				break;
			}
			case ShaderUniformType::Vec2:
			{
				const glm::vec2& value = m_Material->GetVector2(name);
				out << YAML::Key << name << YAML::Value << value;
				break;
			}
			case ShaderUniformType::Vec3:
			{
				const glm::vec3& value = m_Material->GetVector3(name);
				out << YAML::Key << name << YAML::Value << value;
				break;
			}
			case ShaderUniformType::Vec4:
			{
				const glm::vec4& value = m_Material->GetVector4(name);
				out << YAML::Key << name << YAML::Value << value;
				break;
			}
			case ShaderUniformType::Mat2:
			{
				const glm::mat2& value = m_Material->GetMatrix2(name);
				out << YAML::Key << name << YAML::Value << value;
				break;
			}
			case ShaderUniformType::Mat3:
			{
				const glm::mat3& value = m_Material->GetMatrix3(name);
				out << YAML::Key << name << YAML::Value << value;
				break;
			}
			case ShaderUniformType::Mat4:
			{
				const glm::mat4& value = m_Material->GetMatrix4(name);
				out << YAML::Key << name << YAML::Value << value;
				break;
			}
			default:
			{
				ASSERT(false);
				break;
			}
			}
		}

		if (m_Material->GetShader() == "PBRStaticShader")
		{
			out << YAML::Key << "u_AlbedoTexture" << YAML::Value << m_Material->GetTexture("u_AlbedoTexture");
			out << YAML::Key << "u_MetalnessTexture" << YAML::Value << m_Material->GetTexture("u_MetalnessTexture");
			out << YAML::Key << "u_NormalTexture" << YAML::Value << m_Material->GetTexture("u_NormalTexture");
			out << YAML::Key << "u_HeightTexture" << YAML::Value << m_Material->GetTexture("u_HeightTexture");
			out << YAML::Key << "u_OcclusionTexture" << YAML::Value << m_Material->GetTexture("u_OcclusionTexture");
		}
		else if (m_Material->GetShader() == "SkyboxShader")
		{
			out << YAML::Key << "u_SkyboxMap" << YAML::Value << m_Material->GetTexture("u_SkyboxMap");
		}
	}

	bool MaterialSerializer::Deserialize(const std::filesystem::path& path)
	{
		YAML::Node data = YAML::LoadFile(path.string());
		auto root = data["Material"];

		ASSERT(root && root["Shader"]);

		m_Material->SetShader(root["Shader"].as<std::string>());
		if (m_Material->GetShader() == "None")
			return true;
		
		auto shader = LUMEN_ASSET_MANAGER->GetRuntimeAsset<Shader>(m_Material->GetShader());
		const auto& buffer = shader->GetPushConstantBuffers().at("u_Material");
		for (auto& [name, uniform] : buffer.Uniforms)
		{
			switch (uniform.GetType())
			{
			case ShaderUniformType::Bool:
			{
				if (auto value = root[name])
					m_Material->Set(name, value.as<bool>());
				break;
			}
			case ShaderUniformType::Int:
			{
				if (auto value = root[name])
					m_Material->Set(name, value.as<int32_t>());
				break;
			}
			case ShaderUniformType::UInt:
			{
				if (auto value = root[name])
					m_Material->Set(name, value.as<uint32_t>());
				break;
			}
			case ShaderUniformType::Float:
			{
				if (auto value = root[name])
					m_Material->Set(name, value.as<float>());
				break;
			}
			case ShaderUniformType::Vec2:
			{
				if (auto value = root[name])
					m_Material->Set(name, value.as<glm::vec2>());
				break;
			}
			case ShaderUniformType::Vec3:
			{
				if (auto value = root[name])
					m_Material->Set(name, value.as<glm::vec3>());
				break;
			}
			case ShaderUniformType::Vec4:
			{
				if (auto value = root[name])
					m_Material->Set(name, value.as<glm::vec4>());
				break;
			}
			case ShaderUniformType::Mat2:
			{
				if (auto value = root[name])
					m_Material->Set(name, value.as<glm::mat2>());
				break;
			}
			case ShaderUniformType::Mat3:
			{
				if (auto value = root[name])
					m_Material->Set(name, value.as<glm::mat3>());
				break;
			}
			case ShaderUniformType::Mat4:
			{
				if (auto value = root[name])
					m_Material->Set(name, value.as<glm::mat4>());
				break;
			}
			default:
			{
				ASSERT(false);
				break;
			}
			}
		}

		if (m_Material->GetShader() == "PBRStaticShader")
		{
			if (auto value = root["u_AlbedoTexture"])
				m_Material->Set("u_AlbedoTexture", value.as<UUID>());
			else
				m_Material->Set("u_AlbedoTexture", AssetHandle(0));

			if (auto value = root["u_MetalnessTexture"])
				m_Material->Set("u_MetalnessTexture", value.as<UUID>());
			else
				m_Material->Set("u_MetalnessTexture", AssetHandle(0));

			if (auto value = root["u_NormalTexture"])
				m_Material->Set("u_NormalTexture", value.as<UUID>());
			else
				m_Material->Set("u_NormalTexture", AssetHandle(0));

			if (auto value = root["u_HeightTexture"])
				m_Material->Set("u_HeightTexture", value.as<UUID>());
			else
				m_Material->Set("u_HeightTexture", AssetHandle(0));

			if (auto value = root["u_OcclusionTexture"])
				m_Material->Set("u_OcclusionTexture", value.as<UUID>());
			else
				m_Material->Set("u_OcclusionTexture", AssetHandle(0));
		}
		else if (m_Material->GetShader() == "SkyboxShader")
		{
			if(auto value = root["u_SkyboxMap"])
				m_Material->Set("u_SkyboxMap", value.as<UUID>());
			else
				m_Material->Set("u_SkyboxMap", AssetHandle(0));
		}

		return true;
	}
}