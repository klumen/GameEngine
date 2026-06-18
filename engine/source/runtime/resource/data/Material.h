#pragma once

#include "runtime/resource/asset/Asset.h"
#include "runtime/resource/data/Shader.h"
#include "runtime/resource/data/Texture.h"
#include "runtime/core/Macro.h"

#include <unordered_map>
#include <vector>

namespace Lumen
{
	enum class MaterialFlag : uint32_t
	{
		None		= BIT(0),
		DepthTest	= BIT(1),
		Blend		= BIT(2),
		TwoSided	= BIT(3)
	};

	struct MaterialInfo
	{
		// TODO: use asset handle
		std::string Shader = "None";
		uint32_t MaterialFlags = 0;
	};

	class Material : public Asset
	{
	public:
		Material() = default;
		Material(const MaterialInfo& info);
		~Material() = default;

		static AssetType GetStaticType() { return AssetType::Material; }
		virtual AssetType GetType() const override { return GetStaticType(); }

		void SetModified(bool modified = true) { m_Modified = modified; }
		bool GetModified() const { return m_Modified; }

		void SetShader(const std::string& handle);
		
		const std::string& GetShader() const { return m_Info.Shader; }
		uint32_t MaterialFlags() const { return m_Info.MaterialFlags; }

		void Set(const std::string& name, bool value);
		void Set(const std::string& name, int32_t value);
		void Set(const std::string& name, uint32_t value);
		void Set(const std::string& name, float value);
		void Set(const std::string& name, const glm::vec2& value);
		void Set(const std::string& name, const glm::vec3& value);
		void Set(const std::string& name, const glm::vec4& value);
		void Set(const std::string& name, const glm::mat2& value);
		void Set(const std::string& name, const glm::mat3& value);
		void Set(const std::string& name, const glm::mat4& value);
		void Set(const std::string& name, AssetHandle texture);

		template <typename T>
		void Set(const std::string& name, const T& value)
		{
			const auto& decl = FindUniformDeclaration(name);
			ASSERT(decl.GetSize() >= sizeof(value));

			Write(&value, sizeof(value), decl.GetOffset());
		}

		bool& GetBool(const std::string& name);
		int32_t& GetInt(const std::string& name);
		uint32_t& GetUInt(const std::string& name);
		float& GetFloat(const std::string& name);
		glm::vec2& GetVector2(const std::string& name);
		glm::vec3& GetVector3(const std::string& name);
		glm::vec4& GetVector4(const std::string& name);
		glm::mat2& GetMatrix2(const std::string& name);
		glm::mat3& GetMatrix3(const std::string& name);
		glm::mat4& GetMatrix4(const std::string& name);
		AssetHandle& GetTexture(const std::string& name);

		template<typename T>
		T& Get(const std::string& name)
		{
			const auto& decl = FindUniformDeclaration(name);
			return *reinterpret_cast<T*>(Read(decl.GetOffset()));
		}

		void Bind();

	private:
		const ShaderUniform& FindUniformDeclaration(const std::string& name) const;
		const ShaderResourceDeclaration& FindResourceDeclaration(const std::string& name) const;

		void Write(const void* data, uint32_t size, uint32_t offset = 0);
		void* Read(uint32_t offset);

	private:
		MaterialInfo m_Info{};

		std::vector<uint8_t> m_UniformStorageBuffer;
		std::unordered_map<uint32_t, AssetHandle> m_Textures;

		bool m_Modified = true;

	};
}