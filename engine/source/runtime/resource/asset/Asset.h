#pragma once

#include "runtime/core/UUID.h"

namespace Lumen
{
	using AssetHandle = UUID;

	enum class AssetType : uint8_t
	{
		None = 0,
		Default,
		Missing,
		Folder,
		Scene,
		Shader,
		Texture,
		Audio,
		Script,
		Material,
		Model,
		Mesh,
		Animation
	};

	class Asset
	{
	public:
		virtual ~Asset() = default;

		virtual AssetType GetType() const = 0;
		
		static std::string_view AssetTypeToString(AssetType type);
		static AssetType AssetTypeFromString(std::string_view assetType);
	};
}