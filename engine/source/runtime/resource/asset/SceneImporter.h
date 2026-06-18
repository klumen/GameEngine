#pragma once

#include "runtime/resource/asset/AssetImporter.h"
#include "runtime/resource/data/Scene.h"

namespace Lumen
{
	class SceneImporter : public AssetImporter
	{
	public:
		SceneImporter(const std::filesystem::path& scenePath);
		virtual ~SceneImporter() = default;

		virtual Shared<Asset> Import() override;
		virtual bool CreateAsset() override;

		virtual bool Serialize(AssetHandle handle) override;
		virtual AssetHandle Deserialize() override;

	private:

	};
}