#pragma once

#include "runtime/resource/asset/AssetImporter.h"

#include <assimp/scene.h>

namespace Lumen
{
	struct ModelImportSettings
	{
		bool GenerateColliders = false;
		
		std::vector<AssetHandle> Meshes;
		std::vector<AssetHandle> Materials;
		std::vector<AssetHandle> Animations;
	};

	class ModelImporter : public AssetImporter
	{
	public:
		ModelImporter(const std::filesystem::path& path)
			: AssetImporter(path) {}
		virtual ~ModelImporter() = default;

		ModelImportSettings& GetModelImportSettings() { return m_ModelImportSettings; }
		
		virtual Shared<Asset> Import() override;

		virtual bool Serialize(AssetHandle handle) override;
		virtual AssetHandle Deserialize() override;

	private:
		void ProcessMaterials(const aiScene* scene);
		void ProcessNode(aiNode* node, const aiScene* scene);
		void ProcessSubmesh(aiMesh* submesh, const aiScene* scene);

		ModelImportSettings m_ModelImportSettings;

		std::filesystem::path directory;

	};
}