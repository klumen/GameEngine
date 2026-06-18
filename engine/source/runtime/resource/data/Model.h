#pragma once

#include "runtime/resource/asset/Asset.h"

#include <vector>

namespace Lumen
{
	struct ModelInfo
	{
		std::vector<AssetHandle> Meshes;
		std::vector<std::string> MeshesName;

		std::vector<std::vector<AssetHandle>> SubMaterials;

		std::vector<AssetHandle> Materials; // for each submesh
		std::vector<std::string> MaterialsName;
	};

	class Model : public Asset
	{
	public:
		Model() = default;
		Model(const ModelInfo& info) : m_ModelInfo(info) {}
		~Model() = default;

		static AssetType GetStaticType() { return AssetType::Model; }
		virtual AssetType GetType() const override { return GetStaticType(); };

		const std::vector<AssetHandle>& GetMeshes() const { return m_ModelInfo.Meshes; }
		const std::vector<std::string>& GetMeshesName() const { return m_ModelInfo.MeshesName; }
		const std::vector<std::vector<AssetHandle>>& GetSubMaterials() const { return m_ModelInfo.SubMaterials; }
		const std::vector<AssetHandle>& GetMaterials() const { return m_ModelInfo.Materials; }
		const std::vector<std::string>& GetMaterialsName() const { return m_ModelInfo.MaterialsName; }

	private:
		ModelInfo m_ModelInfo;

	};
}