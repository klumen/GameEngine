#pragma once

#include "runtime/resource/data/Shader.h"

#include <spirv_cross/spirv_glsl.hpp>

namespace Lumen
{
	class OpenGLShader : public Shader
	{
	public:
		OpenGLShader(const ShaderInfo& info);
		~OpenGLShader();

		virtual const std::unordered_map<std::string, ShaderPushConstantBuffer>& GetPushConstantBuffers() const override { return m_Buffers; }
		virtual const std::unordered_map<std::string, ShaderResourceDeclaration>& GetResourceDeclarations() const override { return m_Resources; }

		virtual void Bind() const override;
		virtual void Unbind() const override;

		virtual void SetUniformBuffer(const std::string& name, const void* data, uint32_t size, uint32_t offset) override;
		virtual void SetUniform(const std::string& name, bool value) override;
		virtual void SetUniform(const std::string& name, int32_t value) override;
		virtual void SetUniform(const std::string& name, uint32_t value) override;
		virtual void SetUniform(const std::string& name, float value) override;
		virtual void SetUniform(const std::string& name, const glm::vec2& value) override;
		virtual void SetUniform(const std::string& name, const glm::vec3& value) override;
		virtual void SetUniform(const std::string& name, const glm::vec4& value) override;
		virtual void SetUniform(const std::string& name, const glm::mat2& value) override;
		virtual void SetUniform(const std::string& name, const glm::mat3& value) override;
		virtual void SetUniform(const std::string& name, const glm::mat4& value) override;

	private:
		std::vector<uint32_t> CompileSPIRBinary(ShaderType type, const std::string& source);
		uint32_t CompileShader(ShaderType type, const std::vector<uint32_t>& SPIRBinary);
		// void RemapGlslBindings(spirv_cross::CompilerGLSL& compiler);
		void Reflect(const std::vector<uint32_t>& SPIRBinary);

	private:
		uint32_t m_RendererID = 0;

		inline static std::unordered_map<uint32_t, ShaderUniformBuffer> s_UniformBuffers;

		std::unordered_map<std::string, ShaderPushConstantBuffer> m_Buffers;
		std::unordered_map<std::string, ShaderResourceDeclaration> m_Resources;
		std::unordered_map<std::string, int32_t> m_UniformLocations;

	};
}