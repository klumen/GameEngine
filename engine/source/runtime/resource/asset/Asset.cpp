#include "runtime/resource/asset/Asset.h"

namespace Lumen
{
	std::string_view Asset::AssetTypeToString(AssetType type)
	{
		switch (type)
		{
		case Lumen::AssetType::Default:		return "Default";
		case Lumen::AssetType::Missing:		return "Missing";
		case Lumen::AssetType::Folder:		return "Folder";
		case Lumen::AssetType::Scene:		return "Scene";
		case Lumen::AssetType::Shader:		return "Shader";
		case Lumen::AssetType::Texture:		return "Texture";
		case Lumen::AssetType::Audio:		return "Audio";
		case Lumen::AssetType::Script:		return "Script";
		case Lumen::AssetType::Material:	return "Material";
		case Lumen::AssetType::Model:		return "Model";
		case Lumen::AssetType::Mesh:		return "Mesh";
		case Lumen::AssetType::Animation:	return "Animation";
		}

		return "None";
	}

	AssetType Asset::AssetTypeFromString(std::string_view assetType)
	{
		if (assetType == "Default")		return AssetType::Default;
		if (assetType == "Missing")		return AssetType::Missing;
		if (assetType == "Folder")		return AssetType::Folder;
		if (assetType == "Scene")		return AssetType::Scene;
		if (assetType == "Shader")		return AssetType::Shader;
		if (assetType == "Texture")		return AssetType::Texture;
		if (assetType == "Audio")		return AssetType::Audio;
		if (assetType == "Script")		return AssetType::Script;
		if (assetType == "Material")	return AssetType::Material;
		if (assetType == "Model")		return AssetType::Model;
		if (assetType == "Mesh")		return AssetType::Mesh;
		if (assetType == "Animation")	return AssetType::Animation;

		return AssetType::None;
	}
}