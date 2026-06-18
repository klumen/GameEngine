#include "runtime/platform/OpenGL/OpenGLShader.h"

#include "runtime/function/global/GlobalContext.h"
#include "runtime/core/Log.h"
#include "runtime/core/Macro.h"

#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>
#include <shaderc/shaderc.hpp>

#include <unordered_set>

namespace Lumen
{
	static shaderc_shader_kind ShaderTypeToShaderC(ShaderType type)
	{
		switch (type)
		{
		case ShaderType::Vertex:			return shaderc_vertex_shader;
		case ShaderType::Fragment:			return shaderc_fragment_shader;
		case ShaderType::Geometry:			return shaderc_geometry_shader;
		case ShaderType::Compute:			return shaderc_compute_shader;
		case ShaderType::TessControl:		return shaderc_tess_control_shader;
		case ShaderType::TessEvaluation:	return shaderc_tess_evaluation_shader;
		}

		ASSERT(false);
		return shaderc_vertex_shader;
	}

	static GLenum ShaderTypeToGLShaderType(ShaderType type)
	{
		switch (type)
		{
		case ShaderType::Vertex:			return GL_VERTEX_SHADER;
		case ShaderType::Fragment:			return GL_FRAGMENT_SHADER;
		case ShaderType::Geometry:			return GL_GEOMETRY_SHADER;
		case ShaderType::Compute:			return GL_COMPUTE_SHADER;
		case ShaderType::TessControl:		return GL_TESS_CONTROL_SHADER;
		case ShaderType::TessEvaluation:	return GL_TESS_EVALUATION_SHADER;
		}

		ASSERT(false);
		return 0;
	}

	static const char* CachedSPIRVFileExtension(ShaderType type)
	{
		switch (type)
		{
		case ShaderType::Vertex:			return ".cached_spirv.vert";
		case ShaderType::Fragment:			return ".cached_spirv.frag";
		case ShaderType::Geometry:			return ".cached_spirv.geom";
		case ShaderType::Compute:			return ".cached_spirv.comp";
		case ShaderType::TessControl:		return ".cached_spirv.tesc";
		case ShaderType::TessEvaluation:	return ".cached_spirv.tese";
		}

		ASSERT(false);
		return "";
	}

	static const char* CachedOpenGLFileExtension(ShaderType type)
	{
		switch (type)
		{
		case ShaderType::Vertex:			return ".cached_opengl.vert";
		case ShaderType::Fragment:			return ".cached_opengl.frag";
		case ShaderType::Geometry:			return ".cached_opengl.geom";
		case ShaderType::Compute:			return ".cached_opengl.comp";
		case ShaderType::TessControl:		return ".cached_opengl.tesc";
		case ShaderType::TessEvaluation:	return ".cached_opengl.tese";
		}

		ASSERT(false);
		return "";
	}

	static const char* ShaderTypeToString(ShaderType type)
	{
		switch (type)
		{
		case ShaderType::Vertex:			return "Vertex";
		case ShaderType::Fragment:			return "Fragment";
		case ShaderType::Geometry:			return "Geometry";
		case ShaderType::Compute:			return "Compute";
		case ShaderType::TessControl:		return "TessControl";
		case ShaderType::TessEvaluation:	return "TessEvaluation";
		}

		ASSERT(false);
		return "";
	}

	static ShaderUniformType SPIRTypeToShaderUniformType(spirv_cross::SPIRType type)
	{
		switch (type.basetype)
		{
		case spirv_cross::SPIRType::Boolean:  return ShaderUniformType::Bool;
		case spirv_cross::SPIRType::Int:      return ShaderUniformType::Int;
		case spirv_cross::SPIRType::UInt:     return ShaderUniformType::UInt;
		case spirv_cross::SPIRType::Float:
			if (type.vecsize == 1)            return ShaderUniformType::Float;
			if (type.vecsize == 2)            return ShaderUniformType::Vec2;
			if (type.vecsize == 3)            return ShaderUniformType::Vec3;
			if (type.vecsize == 4)            return ShaderUniformType::Vec4;

			if (type.columns == 3)            return ShaderUniformType::Mat3;
			if (type.columns == 4)            return ShaderUniformType::Mat4;
			break;
		}

		ASSERT(false);
		return ShaderUniformType::None;
	}

	OpenGLShader::OpenGLShader(const ShaderInfo& info)
	{
		std::vector<uint32_t> shaders;
		for (const auto& [type, source] : info.shaderSources)
		{
			auto binary = CompileSPIRBinary(type, source);
			auto shader = CompileShader(type, binary);
			shaders.emplace_back(shader);
		}

		m_RendererID = glCreateProgram();
		GLuint program = m_RendererID;

		for (auto& shader : shaders)
			glAttachShader(program, shader);
		glLinkProgram(program);

		GLint isLinked = 0;
		glGetProgramiv(program, GL_LINK_STATUS, &isLinked);
		if (isLinked == GL_FALSE)
		{
			GLint maxLength = 0;
			glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);
			std::vector<char> infoLog(maxLength);
			glGetProgramInfoLog(program, maxLength, &maxLength, infoLog.data());

			for (auto& shader : shaders)
				glDeleteShader(shader);
			glDeleteProgram(program);

			LUMEN_RUNTIME_ERROR("{0}", infoLog.data());
			ASSERT(false);

			return;
		}

		for (auto& shader : shaders)
		{
			glDetachShader(program, shader);
			glDeleteShader(shader);
		}

		for (auto& [bufferName, buffer] : m_Buffers)
		{
			for (auto& [name, uniform] : buffer.Uniforms)
			{
				GLint location = glGetUniformLocation(m_RendererID, name.c_str());
				if (location == -1)
					LUMEN_RUNTIME_WARN("{0}: could not find uniform location {0}", name);

				m_UniformLocations[name] = location;
			}
		}
	}

	OpenGLShader::~OpenGLShader()
	{
		glDeleteProgram(m_RendererID);
	}

	std::vector<uint32_t> OpenGLShader::CompileSPIRBinary(ShaderType type, const std::string& source)
	{
		std::vector<uint32_t> shaderStageData;
		// TODO: get if has cache

		shaderc::Compiler compiler;
		shaderc::CompileOptions options;
		options.SetTargetEnvironment(shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_2);
		options.AddMacroDefinition("OPENGL");
		const bool optimize = false;
		if (optimize)
			options.SetOptimizationLevel(shaderc_optimization_level_performance);

		// Compile shader
		shaderc::SpvCompilationResult module = compiler.CompileGlslToSpv(source, ShaderTypeToShaderC(type), ShaderTypeToString(type), options);

		if (module.GetCompilationStatus() != shaderc_compilation_status_success)
		{
			LUMEN_RUNTIME_ERROR(module.GetErrorMessage());
			ASSERT(false);
		}

		shaderStageData = std::vector<uint32_t>(module.cbegin(), module.cend());

		// TODO: Cache compiled shader
		/*{
			std::filesystem::path p = m_AssetPath;
			auto path = cacheDirectory / (p.filename().string() + extension);
			std::string cachedFilePath = path.string();

			FILE* f = fopen(cachedFilePath.c_str(), "wb");
			fwrite(outputBinary[stage].data(), sizeof(uint32_t), outputBinary[stage].size(), f);
			fclose(f);
		}*/

		Reflect(shaderStageData);

		return shaderStageData;
	}

	uint32_t OpenGLShader::CompileShader(ShaderType type, const std::vector<uint32_t>& SPIRBinary)
	{
		std::vector<uint32_t> shaderStageData;
		// TODO: get if has cache

		spirv_cross::CompilerGLSL glsl(SPIRBinary);
		// RemapGlslBindings(glsl);
		std::string source = glsl.compile();
		LUMEN_RUNTIME_TRACE("=========================================");
		LUMEN_RUNTIME_TRACE("{0} Shader:\n{1}", ShaderTypeToString(type), source.c_str());
		LUMEN_RUNTIME_TRACE("=========================================");

		// TODO: Problem: push_constant to uniform does't own location 
		/*shaderc::Compiler compiler;
		shaderc::CompileOptions options;
		//options.SetAutoMapLocations(true);
		options.SetSuppressWarnings();
		options.SetVulkanRulesRelaxed(true);
		options.SetTargetEnvironment(shaderc_target_env_opengl, shaderc_env_version_opengl_4_5);
		shaderc::SpvCompilationResult module = compiler.CompileGlslToSpv(source, ShaderTypeToShaderC(type), ShaderTypeToString(type), options);

		if (module.GetCompilationStatus() != shaderc_compilation_status_success)
		{
			LUMEN_RUNTIME_ERROR(module.GetErrorMessage());
			ASSERT(false);
		}

		shaderStageData = std::vector<uint32_t>(module.cbegin(), module.cend());*/
					
		// TODO: cache
		/*{
			std::filesystem::path p = m_AssetPath;
			auto path = cacheDirectory / (p.filename().string() + GLShaderStageCachedOpenGLFileExtension(stage));
			std::string cachedFilePath = path.string();
			FILE* f = fopen(cachedFilePath.c_str(), "wb");
			fwrite(shaderStageData.data(), sizeof(uint32_t), shaderStageData.size(), f);
			fclose(f);
		}*/

		GLuint shader = glCreateShader(ShaderTypeToGLShaderType(type));
		const char* src = source.c_str();
		glShaderSource(shader, 1, &src, nullptr);
		glCompileShader(shader);
		/*glShaderBinary(1, &shader, GL_SHADER_BINARY_FORMAT_SPIR_V, shaderStageData.data(), static_cast<uint32_t>(shaderStageData.size() * sizeof(uint32_t)));
		glSpecializeShader(shader, "main", 0, nullptr, nullptr);*/

		GLint isCompiled = 0;
		glGetShaderiv(shader, GL_COMPILE_STATUS, &isCompiled);
		if (isCompiled == GL_FALSE)
		{
			GLint maxLength = 0;
			glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);
			std::vector<char> infoLog(maxLength);
			glGetShaderInfoLog(shader, maxLength, &maxLength, infoLog.data());

			glDeleteShader(shader);

			LUMEN_RUNTIME_ERROR("{}", infoLog.data());
			ASSERT(false);
			
			return 0;
		}

		return shader;
	}

	/*void OpenGLShader::RemapGlslBindings(spirv_cross::CompilerGLSL& compiler)
	{
		auto resources = compiler.get_shader_resources();
		uint32_t binding = 0;

		for (auto& ubo : resources.uniform_buffers)
			compiler.set_decoration(ubo.id, spv::DecorationBinding, binding++);

		for (auto& sampler : resources.sampled_images)
			compiler.set_decoration(sampler.id, spv::DecorationBinding, binding++);

		for (auto& ssbo : resources.storage_buffers)
			compiler.set_decoration(ssbo.id, spv::DecorationBinding, binding++);
	}*/

	void OpenGLShader::Reflect(const std::vector<uint32_t>& SPIRBinary)
	{
		LUMEN_RUNTIME_TRACE("===========================");
		spirv_cross::Compiler compiler(SPIRBinary);
		auto resources = compiler.get_shader_resources();

		uint32_t bufferOffset = 0;
		LUMEN_RUNTIME_TRACE("Push Constant Buffers:");
		for (const spirv_cross::Resource& resource : resources.push_constant_buffers)
		{
			const auto& bufferName = resource.name;
			const auto& bufferType = compiler.get_type(resource.base_type_id);
			uint32_t bufferSize = static_cast<uint32_t>(compiler.get_declared_struct_size(bufferType));

			// Skip empty push constant buffers (or these are for the renderer only)
			if (bufferName.empty())
			{
				bufferOffset += bufferSize;
				continue;
			}

			auto location = compiler.get_decoration(resource.id, spv::DecorationLocation);
			uint32_t memberCount = static_cast<uint32_t>(bufferType.member_types.size());
			ShaderPushConstantBuffer& buffer = m_Buffers[bufferName];
			buffer.Name = bufferName;
			buffer.Size = bufferSize - bufferOffset;
			for (uint32_t i = 0; i < memberCount; i++)
			{
				const auto& type = compiler.get_type(bufferType.member_types[i]);
				const auto& memberName = compiler.get_member_name(bufferType.self, i);
				uint32_t size = static_cast<uint32_t>(compiler.get_declared_struct_member_size(bufferType, i));
				uint32_t offset = compiler.type_struct_member_offset(bufferType, i) - bufferOffset;

				std::string uniformName = bufferName + "." + memberName;
				buffer.Uniforms[uniformName] = ShaderUniform(uniformName, SPIRTypeToShaderUniformType(type), size, offset);
			}

			LUMEN_RUNTIME_TRACE("  Name: {0}", bufferName);
			LUMEN_RUNTIME_TRACE("  Member Count: {0}", memberCount);
			LUMEN_RUNTIME_TRACE("  Size: {0}", bufferSize);
		}

		LUMEN_RUNTIME_TRACE("Uniform Buffers:");
		for (const spirv_cross::Resource& resource : resources.uniform_buffers)
		{
			const auto& bufferType = compiler.get_type(resource.base_type_id);
			uint32_t binding = compiler.get_decoration(resource.id, spv::DecorationBinding);
			uint32_t bufferSize = static_cast<uint32_t>(compiler.get_declared_struct_size(bufferType));
			uint32_t memberCount = static_cast<uint32_t>(bufferType.member_types.size());

			LUMEN_RUNTIME_TRACE("  {0} ({1})", resource.name, binding);
			LUMEN_RUNTIME_TRACE("  Member Count: {0}", memberCount);
			LUMEN_RUNTIME_TRACE("  Size: {0}", bufferSize);

			if (s_UniformBuffers.find(binding) == s_UniformBuffers.end())
			{
				ShaderUniformBuffer& buffer = s_UniformBuffers[binding];
				buffer.Name = resource.name;
				buffer.BindingPoint = binding;
				buffer.Size = bufferSize;
				glCreateBuffers(1, &buffer.RendererID);
				glBindBuffer(GL_UNIFORM_BUFFER, buffer.RendererID);
				glBufferData(GL_UNIFORM_BUFFER, buffer.Size, nullptr, GL_DYNAMIC_DRAW);
				glBindBufferBase(GL_UNIFORM_BUFFER, buffer.BindingPoint, buffer.RendererID);

				LUMEN_RUNTIME_TRACE("  Created Uniform Buffer at binding point {0} with name '{1}', size is {2} bytes", buffer.BindingPoint, buffer.Name, buffer.Size);

				glBindBuffer(GL_UNIFORM_BUFFER, 0);
			}
			else
			{
				// Validation
				ShaderUniformBuffer& buffer = s_UniformBuffers.at(binding);
				ASSERT(buffer.Name == resource.name); // Must be the same buffer
				if (bufferSize > buffer.Size) // Resize buffer if needed
				{
					buffer.Size = bufferSize;

					glDeleteBuffers(1, &buffer.RendererID);
					glCreateBuffers(1, &buffer.RendererID);
					glBindBuffer(GL_UNIFORM_BUFFER, buffer.RendererID);
					glBufferData(GL_UNIFORM_BUFFER, buffer.Size, nullptr, GL_DYNAMIC_DRAW);
					glBindBufferBase(GL_UNIFORM_BUFFER, buffer.BindingPoint, buffer.RendererID);

					LUMEN_RUNTIME_TRACE("Resized Uniform Buffer at binding point {0} with name '{1}', size is {2} bytes", buffer.BindingPoint, buffer.Name, buffer.Size);
				}
			}
		}

		// TODO: SSBO
		
		LUMEN_RUNTIME_TRACE("Sampled Images:");
		for (const spirv_cross::Resource& resource : resources.sampled_images)
		{
			auto& type = compiler.get_type(resource.base_type_id);
			auto binding = compiler.get_decoration(resource.id, spv::DecorationBinding);
			const auto& name = resource.name;
			uint32_t dimension = type.image.dim;

			m_Resources[name] = ShaderResourceDeclaration(name, binding, 1);

			LUMEN_RUNTIME_TRACE("  {0} ({1})", name, binding);
		}

		LUMEN_RUNTIME_TRACE("===========================");
	}

	void OpenGLShader::Bind() const
	{
		glUseProgram(m_RendererID);
	}

	void OpenGLShader::Unbind() const
	{
		glUseProgram(0);
	}

	void OpenGLShader::SetUniformBuffer(const std::string& name, const void* data, uint32_t size, uint32_t offset)
	{
		ShaderUniformBuffer* uniformBuffer = nullptr;
		for (auto& [bindingPoint, ub] : s_UniformBuffers)
		{
			if (ub.Name == name)
			{
				uniformBuffer = &ub;
				break;
			}
		}

		ASSERT(uniformBuffer);
		ASSERT(uniformBuffer->Size >= size);
		glNamedBufferSubData(uniformBuffer->RendererID, offset, size, data);
	}

	void OpenGLShader::SetUniform(const std::string& name, bool value)
	{
		glUniform1ui(m_UniformLocations.at(name), uint32_t(value));
	}

	void OpenGLShader::SetUniform(const std::string& name, int32_t value)
	{
		glUniform1i(m_UniformLocations.at(name), value);
	}

	void OpenGLShader::SetUniform(const std::string& name, uint32_t value)
	{
		glUniform1ui(m_UniformLocations.at(name), value);
	}

	void OpenGLShader::SetUniform(const std::string& name, float value)
	{
		glUniform1f(m_UniformLocations.at(name), value);
	}

	void OpenGLShader::SetUniform(const std::string& name, const glm::vec2& value)
	{
		glUniform2fv(m_UniformLocations.at(name), 1, glm::value_ptr(value));
	}

	void OpenGLShader::SetUniform(const std::string& name, const glm::vec3& value)
	{
		glUniform3fv(m_UniformLocations.at(name), 1, glm::value_ptr(value));
	}

	void OpenGLShader::SetUniform(const std::string& name, const glm::vec4& value)
	{
		glUniform4fv(m_UniformLocations.at(name), 1, glm::value_ptr(value));
	}

	void OpenGLShader::SetUniform(const std::string& name, const glm::mat2& value)
	{
		glUniformMatrix2fv(m_UniformLocations.at(name), 1, GL_FALSE, glm::value_ptr(value));
	}

	void OpenGLShader::SetUniform(const std::string& name, const glm::mat3& value)
	{
		glUniformMatrix3fv(m_UniformLocations.at(name), 1, GL_FALSE, glm::value_ptr(value));
	}

	void OpenGLShader::SetUniform(const std::string& name, const glm::mat4& value)
	{
		glUniformMatrix4fv(m_UniformLocations.at(name), 1, GL_FALSE, glm::value_ptr(value));
	}
}