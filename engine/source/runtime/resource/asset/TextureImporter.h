#pragma once

#include "runtime/resource/asset/AssetImporter.h"
#include "runtime/resource/data/Texture.h"

namespace Lumen
{
	struct TextureImportSettings
	{
		ImageShape Shape = ImageShape::Texture2D;
		bool sRGB = false;
		bool GenerateMipMaps = false;
	};

	class TextureImporter : public AssetImporter
	{
	public:
		TextureImporter(const std::filesystem::path& texturePath);
		virtual ~TextureImporter() = default;

		TextureImportSettings& GetTextureImportSettings() { return m_TextureImportSettings; }
		
		virtual Shared<Asset> Import() override;

		virtual bool Serialize(AssetHandle handle) override;
		virtual AssetHandle Deserialize() override;

	private:
		TextureImportSettings m_TextureImportSettings;

	};
}