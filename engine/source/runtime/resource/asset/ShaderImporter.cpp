#include "runtime/resource/asset/ShaderImporter.h"

#include "runtime/function/global/GlobalContext.h"
#include "runtime/resource/data/Shader.h"
#include "runtime/resource/asset/Serializer.h"
#include "runtime/core/Log.h"
#include "runtime/core/Macro.h"

namespace Lumen
{
	static ShaderType ShaderTypeFromString(std::string_view type)
	{
		if (type == "Vertex")			return ShaderType::Vertex;
		if (type == "Fragment")			return ShaderType::Fragment;
		if (type == "Geometry")			return ShaderType::Geometry;
		if (type == "Compute")			return ShaderType::Compute;
		if (type == "TessControl")		return ShaderType::TessControl;
		if (type == "TessEvaluation")	return ShaderType::TessEvaluation;

		ASSERT(false);
		return ShaderType::None;
	}

	ShaderImporter::ShaderImporter(const std::filesystem::path& shaderPath)
		: AssetImporter(shaderPath) { }

	Shared<Asset> ShaderImporter::Import()
	{
		/*switch (LUMEN_GRAPHICS_API)
		{
		case GraphicsAPI::None: ASSERT(false); break;
		case GraphicsAPI::OpenGL:
			LUMEN_FILE_SYSTEM->CreateDirectories(
				LUMEN_CONFIG_MANAGER->GetCacheDir() / "shaders/opengl");
			break;
		case GraphicsAPI::Vulkan:
			LUMEN_FILE_SYSTEM->CreateDirectories(
				LUMEN_CONFIG_MANAGER->GetCacheDir() / "shaders/vulkan");
			break;
		}*/

		std::string shaderCode;
		std::ifstream shaderFile(m_AssetPath);
		if (shaderFile.is_open())
		{
			std::stringstream shaderString;
			shaderString << shaderFile.rdbuf();
			shaderCode = shaderString.str();
			shaderFile.close();
		}
		else
		{
			LUMEN_RUNTIME_ERROR("ShaderImporter::Import::Unable to open the file {0}", m_AssetPath.string());
			return nullptr;
		}

		const char* typeToken = "#type";
		const size_t typeTokenLength = strlen(typeToken);
		size_t pos = shaderCode.find(typeToken, 0);

		std::unordered_map<ShaderType, std::string> shaderSources;
		while (pos != std::string::npos)
		{
			size_t eol = shaderCode.find_first_of("\r\n", pos); //End of shader type declaration line
			ASSERT(eol != std::string::npos);
			
			size_t begin = pos + typeTokenLength + 1; //Start of shader type name (after "#type " keyword)
			std::string type = shaderCode.substr(begin, eol - begin);

			size_t nextLinePos = shaderCode.find_first_not_of("\r\n", eol); //Start of shader code after shader type declaration line
			ASSERT(nextLinePos != std::string::npos);
			
			pos = shaderCode.find(typeToken, nextLinePos); //Start of next shader type declaration line

			shaderSources[ShaderTypeFromString(type)] = 
				(pos == std::string::npos) 
				? shaderCode.substr(nextLinePos) 
				: shaderCode.substr(nextLinePos, pos - nextLinePos);
		}

		ShaderInfo info;
		info.shaderSources = std::move(shaderSources);
		return Shader::Create(info);
	}

	bool ShaderImporter::CreateAsset()
	{

		return false;
	}
}