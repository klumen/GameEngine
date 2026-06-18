#pragma once

#include "runtime/resource/asset/Asset.h"

#include <filesystem>

namespace Lumen
{
	struct AssetMetadata
	{
		AssetType type = AssetType::None;
		std::filesystem::path assetFilePath; // relative to asset dir path

		operator bool() const { return type != AssetType::None; }
	};
}