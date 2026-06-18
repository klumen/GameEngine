#pragma once

#include "runtime/resource/asset/Asset.h"
#include "runtime/core/Memory.h"

#include <filesystem>

namespace Lumen
{
	class AssetImporter
	{
	public:
		AssetImporter(const std::filesystem::path& assetPath);
		virtual ~AssetImporter() = default;

		static Shared<AssetImporter> Create(AssetType type, const std::filesystem::path& path);

		virtual Shared<Asset> Import() = 0;
		virtual bool CreateAsset() { return false; };
		virtual bool Serialize(AssetHandle handle) { return false; }
		virtual AssetHandle Deserialize() { return 0; }

	protected:
		std::filesystem::path m_AssetPath; // absolute path

	};
}