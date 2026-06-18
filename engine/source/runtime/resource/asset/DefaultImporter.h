#pragma once

#include "runtime/resource/asset/AssetImporter.h"

namespace Lumen
{
	class DefaultImporter : public AssetImporter
	{
	public:
		DefaultImporter(const std::filesystem::path& path);
		~DefaultImporter() = default;

		virtual Shared<Asset> Import() override;
		virtual bool Serialize(AssetHandle handle) override;
		virtual AssetHandle Deserialize() override;

	private:

	};
}