#pragma once

#include "runtime/resource/asset/AssetImporter.h"
#include "runtime/resource/data/Scene.h"
#include "runtime/resource/data/Material.h"
#include "runtime/resource/data/Texture.h"
#include "runtime/resource/data/Shader.h"
#include "runtime/resource/asset/AssetMetadata.h"
#include "runtime/core/Memory.h"
#include "runtime/core/Macro.h"

#include <unordered_map>

namespace Lumen
{
	using AssetRegistry = std::unordered_map<AssetHandle, AssetMetadata>;
	using AssetMap = std::unordered_map<AssetHandle, Shared<Asset>>;
	using RuntimeMap = std::unordered_map<std::string, Shared<Asset>>;

	// TODO: runtime AssetManager with asset pack
	class AssetManager
	{
	public:
		AssetManager() {};
		~AssetManager() {};

		void StartUp();
		void ShutDown();

		// editor
		inline const std::filesystem::path& GetAssetDirectory() { return m_AssetDirectory; }
		inline const std::filesystem::path& GetAssetRegistryPath() { return m_AssetRegistryPath; }
		inline void SetAssetDirectory(const std::filesystem::path& path) { m_AssetDirectory = path; }
		inline void SetAssetRegistryPath(const std::filesystem::path& path) { m_AssetRegistryPath = path; }

		bool SerializeAssetRegistry(const std::filesystem::path& path);
		bool DeserializeAssetRegistry(const std::filesystem::path& path);

		// relative to asset dir (register assets)
		bool ImportAssets(const std::filesystem::path& assetPath);
		void AddRegisterAsset(AssetHandle handle, const AssetMetadata& metadata);
		void AddLoadedAsset(AssetHandle handle, const Shared<Asset>& asset);

		// relative to asset dir
		AssetHandle GetHandle(const std::filesystem::path& assetPath) const;
		AssetType GetAssetType(AssetHandle handle) const;
		// relative to the asset dir
		const std::filesystem::path& GetAssetPath(AssetHandle handle) const;
		std::filesystem::path GetAssetName(AssetHandle handle);

		void UpdateRuntimeAsset(const std::string& name, const Shared<Asset>& asset);

		Shared<Asset> GetRuntimeAsset(const std::string& name) const;
		template<typename T>
		auto GetRuntimeAsset(const std::string& name)
		{
			return std::dynamic_pointer_cast<T>(GetRuntimeAsset(name));
		}

		bool LoadAsset(AssetHandle handle);

		Shared<Asset> GetAsset(AssetHandle handle);
		template<typename T>
		auto GetAsset(AssetHandle handle)
		{
			return std::dynamic_pointer_cast<T>(GetAsset(handle));
		}

		void UpdateAsset(AssetHandle handle, Shared<Asset> asset);

		template<typename T, typename ...Args>
		AssetHandle CreateAsset(const std::string& filename, AssetHandle directory = 0, Args&& ...args)
		{
			STATIC_ASSERT(std::is_base_of<Asset, T>::value, "CreateAsset only works for types derived from Asset");

			AssetHandle handle;
			AssetMetadata metadata;
			metadata.type = T::GetStaticType();

			if (directory)
			{
				if (GetAssetType(directory) != AssetType::Folder)
					return 0;
				
				metadata.assetFilePath = GetAssetPath(directory) / filename;
			}
			else
				metadata.assetFilePath = filename;

			auto importer = AssetImporter::Create(metadata.type, m_AssetDirectory / metadata.assetFilePath);
			if (importer->CreateAsset())
			{
				auto asset = importer->Import();
				importer->Serialize(handle);

				m_AssetRegistry[handle] = metadata;
				m_LoadedAssets[handle] = asset;
				SerializeAssetRegistry(m_AssetRegistryPath);
				return handle;
			}

			return 0;
		}
		bool RenameAsset(AssetHandle handle, const std::filesystem::path& name);
		bool MoveAsset(AssetHandle handle, AssetHandle to);
		bool CopyAsset(AssetHandle handle, AssetHandle to);
		bool DeleteAsset(AssetHandle handle);

	private:
		AssetManager(const AssetManager&) = delete;
		AssetManager& operator=(const AssetManager&) = delete;

		std::filesystem::path m_AssetDirectory;
		std::filesystem::path m_AssetRegistryPath;

		AssetRegistry m_AssetRegistry;
		AssetMap m_LoadedAssets;
		RuntimeMap m_RuntimeAsset;

	};
}