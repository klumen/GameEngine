#pragma once

#include "runtime/resource/asset/AssetImporter.h"

namespace Lumen
{
	class MaterialImporter : public AssetImporter
	{
	public:
		MaterialImporter(const std::filesystem::path& path);
		virtual ~MaterialImporter() = default;

		virtual Shared<Asset> Import() override;
		virtual bool CreateAsset() override;
		virtual bool Serialize(AssetHandle handle) override;
		virtual AssetHandle Deserialize() override;

	private:

	};
}