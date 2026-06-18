#include "runtime/resource/asset/AssetImporter.h"

#include "runtime/resource/asset/DefaultImporter.h"
#include "runtime/resource/asset/FolderImporter.h"
#include "runtime/resource/asset/ShaderImporter.h"
#include "runtime/resource/asset/TextureImporter.h"
#include "runtime/resource/asset/ModelImporter.h"
#include "runtime/resource/asset/SceneImporter.h"
#include "runtime/resource/asset/MaterialImporter.h"
#include "runtime/core/Macro.h"

namespace Lumen
{
	AssetImporter::AssetImporter(const std::filesystem::path& assetPath)
		: m_AssetPath(assetPath) { }

	Shared<AssetImporter> AssetImporter::Create(AssetType type, const std::filesystem::path& path)
	{
		switch (type)
		{
		case AssetType::None:		ASSERT(false); return nullptr;
		case AssetType::Default:	return MakeShared<DefaultImporter>(path);
		case AssetType::Folder:		return MakeShared<FolderImporter>(path);
		case AssetType::Shader:		return MakeShared<ShaderImporter>(path);
		case AssetType::Scene:		return MakeShared<SceneImporter>(path);
		case AssetType::Texture:	return MakeShared<TextureImporter>(path);
		case AssetType::Model:		return MakeShared<ModelImporter>(path);
		case AssetType::Material:	return MakeShared<MaterialImporter>(path);
		}

		ASSERT(false);
		return nullptr;
	}
}