#pragma once

#include "runtime/resource/asset/Asset.h"
#include "runtime/resource/data/ShaderDataType.h"
#include "runtime/core/Memory.h"

#include <glm/glm.hpp>

#include <filesystem>
#include <unordered_map>
#include <vector>

namespace Lumen
{
	enum class ShaderType : uint8_t
	{
		None = 0,

		Vertex,
		Fragment,
		Geometry,
		Compute,
		TessControl,
		TessEvaluation
	};

	enum class ShaderUniformType
	{
		None = 0, Bool, Int, UInt, Float, Vec2, Vec3, Vec4, Mat2, Mat3, Mat4
	};

	class ShaderUniform
	{
	public:
		ShaderUniform() = default;
		ShaderUniform(const std::string& name, ShaderUniformType type, uint32_t size, uint32_t offset)
			: m_Name(name), m_Type(type), m_Size(size), m_Offset(offset) {}

		const std::string& GetName() const { return m_Name; }
		ShaderUniformType GetType() const { return m_Type; }
		uint32_t GetSize() const { return m_Size; }
		uint32_t GetOffset() const { return m_Offset; }

		static const std::string& UniformTypeToString(ShaderUniformType type)
		{
			switch (type)
			{
			case Lumen::ShaderUniformType::None:
				break;
			case Lumen::ShaderUniformType::Bool:
				return "Boolean";
				break;
			case Lumen::ShaderUniformType::Int:
				return "Int";
				break;
			case Lumen::ShaderUniformType::UInt:
				break;
			case Lumen::ShaderUniformType::Float:
				return "Float";
				break;
			case Lumen::ShaderUniformType::Vec2:
				break;
			case Lumen::ShaderUniformType::Vec3:
				break;
			case Lumen::ShaderUniformType::Vec4:
				break;
			case Lumen::ShaderUniformType::Mat3:
				break;
			case Lumen::ShaderUniformType::Mat4:
				break;
			}

			return "None";
		}

	private:
		std::string m_Name;
		ShaderUniformType m_Type = ShaderUniformType::None;
		uint32_t m_Size = 0;
		uint32_t m_Offset = 0;
	};

	struct ShaderUniformBuffer
	{
		std::string Name;
		uint32_t Index;
		uint32_t BindingPoint;
		uint32_t Size;
		uint32_t RendererID;
		std::vector<ShaderUniform> Uniforms;
	};

	struct ShaderPushConstantBuffer
	{
		std::string Name;
		uint32_t Size = 0;
		std::unordered_map<std::string, ShaderUniform> Uniforms;
	};

	class ShaderResourceDeclaration
	{
	public:
		ShaderResourceDeclaration() = default;
		ShaderResourceDeclaration(const std::string& name, uint32_t resourceRegister, uint32_t count)
			: m_Name(name), m_Register(resourceRegister), m_Count(count) { }

		const std::string& GetName() const { return m_Name; }
		uint32_t GetRegister() const { return m_Register; }
		uint32_t GetCount() const { return m_Count; }

	private:
		std::string m_Name;
		uint32_t m_Register = 0;
		uint32_t m_Count = 0;
	};

	struct ShaderInfo
	{
		std::unordered_map<ShaderType, std::string> shaderSources;
	};

	class Shader : public Asset
	{
	public:
		virtual ~Shader() = default;

		static Shared<Shader> Create(const ShaderInfo& info);

		static AssetType GetStaticType() { return AssetType::Shader; }
		virtual AssetType GetType() const override { return GetStaticType(); };

		virtual const std::unordered_map<std::string, ShaderPushConstantBuffer>& GetPushConstantBuffers() const = 0;
		virtual const std::unordered_map<std::string, ShaderResourceDeclaration>& GetResourceDeclarations() const = 0;

		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;

		virtual void SetUniformBuffer(const std::string& name, const void* data, uint32_t size, uint32_t offset) = 0;
		virtual void SetUniform(const std::string& name, bool value) = 0;
		virtual void SetUniform(const std::string& name, int32_t value) = 0;
		virtual void SetUniform(const std::string& name, uint32_t value) = 0;
		virtual void SetUniform(const std::string& name, float value) = 0;
		virtual void SetUniform(const std::string& name, const glm::vec2& value) = 0;
		virtual void SetUniform(const std::string& name, const glm::vec3& value) = 0;
		virtual void SetUniform(const std::string& name, const glm::vec4& value) = 0;
		virtual void SetUniform(const std::string& name, const glm::mat2& value) = 0;
		virtual void SetUniform(const std::string& name, const glm::mat3& value) = 0;
		virtual void SetUniform(const std::string& name, const glm::mat4& value) = 0;
	};
}