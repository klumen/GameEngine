#include "runtime/resource/asset/TextureImporter.h"

#include "runtime/function/global/GlobalContext.h"
#include "runtime/resource/asset/Serializer.h"
#include "runtime/core/Log.h"
#include "runtime/core/Macro.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace Lumen
{
	TextureImporter::TextureImporter(const std::filesystem::path& texturePath)
		: AssetImporter(texturePath) { }

	Shared<Asset> TextureImporter::Import()
	{
		int width, height, channels;
		uint8_t* buffer = nullptr;
		uint32_t imageSize = 0;
		std::vector<uint8_t> data;
		bool isHDR = false;

		stbi_set_flip_vertically_on_load(1);
		if (isHDR = stbi_is_hdr(m_AssetPath.string().c_str()))
		{
			LUMEN_RUNTIME_INFO("Loading HDR texture: {0}", m_AssetPath.generic_string());
			buffer = reinterpret_cast<uint8_t*>(stbi_loadf(m_AssetPath.string().c_str(), &width, &height, &channels, 0));
			imageSize = sizeof(float) * width * height * channels;
		}
		else
		{
			LUMEN_RUNTIME_INFO("Loading texture: {0}", m_AssetPath.generic_string());
			buffer = stbi_load(m_AssetPath.string().c_str(), &width, &height, &channels, 0);
			imageSize = width * height * channels;
		}

		if (buffer)
		{
			data.resize(imageSize);
			std::copy(buffer, buffer + imageSize, data.begin());
			stbi_image_free(buffer);
		}
		else
		{
			return nullptr;
		}

		TextureInfo info;
		info.width = width;
		info.height = height;
		info.isHDR = isHDR;
		info.shape = m_TextureImportSettings.Shape;
		info.generateMipmaps = m_TextureImportSettings.GenerateMipMaps;
		switch (channels)
		{
		case 1:
			info.format = info.isHDR ? ImageFormat::R16F : ImageFormat::R8;
			break;
		case 2:
			info.format = info.isHDR ? ImageFormat::RG16F : ImageFormat::RG8;
			break;
		case 3:
			info.format = info.isHDR ? ImageFormat::RGB16F : 
				m_TextureImportSettings.sRGB ? ImageFormat::SRGB8 : ImageFormat::RGB8;
			break;
		case 4:
			info.format = info.isHDR ? ImageFormat::RGBA16F : 
				m_TextureImportSettings.sRGB ? ImageFormat::SRGBA8 : ImageFormat::RGBA8;
			break;
		}

		return Texture::Create(info, data);
	}

	bool TextureImporter::Serialize(AssetHandle handle)
	{
		YAML::Emitter out;

		out << YAML::BeginMap;
		out << YAML::Key << "UUID" << YAML::Value << handle;
		out << YAML::Key << "TextureImporter" << YAML::Value;
		{
			out << YAML::BeginMap;
			out << YAML::Key << "Shape" << YAML::Value << uint32_t(m_TextureImportSettings.Shape);
			out << YAML::Key << "sRGB" << YAML::Value << m_TextureImportSettings.sRGB;
			out << YAML::Key << "GenerateMipMaps" << YAML::Value << m_TextureImportSettings.GenerateMipMaps;
			out << YAML::EndMap;
		}
		out << YAML::EndMap;

		std::ofstream fout(m_AssetPath.string() + ".meta");
		ASSERT(fout.is_open());
		fout << out.c_str();

		return true;
	}

	AssetHandle TextureImporter::Deserialize()
	{
		auto data = YAML::LoadFile(m_AssetPath.string() + ".meta");
		ASSERT(data);
		
		AssetHandle handle = data["UUID"].as<UUID>();
		auto settings = data["TextureImporter"];
		ASSERT(settings);

		if (auto node = settings["Shape"])
			m_TextureImportSettings.Shape = ImageShape(node.as<uint32_t>());
		if (auto node = settings["sRGB"])
			m_TextureImportSettings.sRGB = node.as<bool>();
		if (auto node = settings["GenerateMipMaps"])
			m_TextureImportSettings.GenerateMipMaps = node.as<bool>();

		return handle;
	}
}