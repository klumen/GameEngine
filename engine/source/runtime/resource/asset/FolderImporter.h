#pragma once

#include "runtime/resource/asset/AssetImporter.h"

namespace Lumen
{
	class FolderImporter : public AssetImporter
	{
	public:
		FolderImporter(const std::filesystem::path& path);
		virtual ~FolderImporter() = default;

		virtual Shared<Asset> Import() override;
		virtual bool CreateAsset() override;
		virtual bool Serialize(AssetHandle handle) override;
		virtual AssetHandle Deserialize() override;

	private:

	};
}