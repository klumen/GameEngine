#include "runtime/resource/asset/AssetManager.h"

#include "runtime/function/global/GlobalContext.h"
#include "runtime/resource/asset/AssetImporter.h"
#include "runtime/resource/asset/ModelImporter.h"
#include "runtime/resource/data/Model.h"
#include "runtime/resource/asset/Serializer.h"
#include "runtime/core/Log.h"
#include "runtime/core/Macro.h"

namespace Lumen
{
	static std::unordered_map<std::filesystem::path, AssetType> s_AssetExtensionMap = {
	{ ".mat",	AssetType::Material },
	{ ".glsl",	AssetType::Shader },
	{ ".fbx",	AssetType::Model },
	{ ".FBX",	AssetType::Model },
	{ ".obj",	AssetType::Model },
	{ ".tga",	AssetType::Texture },
	{ ".bmp",	AssetType::Texture },
	{ ".png",	AssetType::Texture },
	{ ".jpg",	AssetType::Texture },
	{ ".jpeg",	AssetType::Texture },
	{ ".hdr",	AssetType::Texture } };

	static AssetType GetAssetTypeFromFileExtension(const std::filesystem::path& extension)
	{
		if (s_AssetExtensionMap.find(extension) == s_AssetExtensionMap.end())
		{
			LUMEN_RUNTIME_WARN("Could not find AssetType for {}", extension.string());
			return AssetType::Default;
		}

		return s_AssetExtensionMap.at(extension);
	}

	void AssetManager::StartUp()
	{
	}

	void AssetManager::ShutDown()
	{
	}

	bool AssetManager::SerializeAssetRegistry(const std::filesystem::path& path)
	{
		YAML::Emitter out;
		{
			out << YAML::BeginMap; // Root
			out << YAML::Key << "AssetRegistry" << YAML::Value;

			out << YAML::BeginSeq;
			for (const auto& [handle, metadata] : m_AssetRegistry)
			{
				out << YAML::BeginMap;
				out << YAML::Key << "Handle" << YAML::Value << handle;
				std::string filepathStr = metadata.assetFilePath.generic_string();
				out << YAML::Key << "FilePath" << YAML::Value << filepathStr;
				out << YAML::Key << "Type" << YAML::Value << Asset::AssetTypeToString(metadata.type).data();
				out << YAML::EndMap;
			}
			out << YAML::EndSeq;
			out << YAML::EndMap; // Root
		}

		std::ofstream fout(path);
		ASSERT(fout.is_open());
		fout << out.c_str();

		return true;
	}

	bool AssetManager::DeserializeAssetRegistry(const std::filesystem::path& path)
	{
		m_AssetRegistry.clear();
		m_LoadedAssets.clear();

		YAML::Node data;
		data = YAML::LoadFile(path.string());
		auto rootNode = data["AssetRegistry"];
		ASSERT(rootNode);

		for (const auto& node : rootNode)
		{
			AssetHandle handle = node["Handle"].as<uint64_t>();
			auto& metadata = m_AssetRegistry[handle];
			metadata.assetFilePath = node["FilePath"].as<std::string>();
			metadata.type = Asset::AssetTypeFromString(node["Type"].as<std::string>());
		}

		return true;
	}

	bool AssetManager::ImportAssets(const std::filesystem::path& assetPath)
	{
		AssetHandle handle;
		AssetMetadata metadata;
		metadata.assetFilePath = assetPath;

		std::filesystem::path fullPath = m_AssetDirectory / assetPath;
		if (std::filesystem::is_directory(fullPath))
		{
			metadata.type = AssetType::Folder;
			for (auto& entry : std::filesystem::directory_iterator(fullPath))
			{
				if (entry.path().extension() == ".meta")
					continue;
				ImportAssets(std::filesystem::relative(entry, m_AssetDirectory));
			}
		}
		else
			metadata.type = GetAssetTypeFromFileExtension(assetPath.extension());

		if (metadata.type == AssetType::None)
			return false;

		auto importer = AssetImporter::Create(metadata.type, fullPath);
		importer->Serialize(handle);

		m_AssetRegistry[handle] = metadata;
		// TODO: reduce serialize time
		SerializeAssetRegistry(m_AssetRegistryPath);
		
		return true;
	}

	void AssetManager::AddRegisterAsset(AssetHandle handle, const AssetMetadata& metadata)
	{
		ASSERT(handle);

		m_AssetRegistry[handle] = metadata;
		SerializeAssetRegistry(m_AssetRegistryPath);
	}

	void AssetManager::AddLoadedAsset(AssetHandle handle, const Shared<Asset>& asset)
	{
		ASSERT(handle && m_AssetRegistry.find(handle) != m_AssetRegistry.end());

		m_LoadedAssets[handle] = asset;
	}

	AssetHandle AssetManager::GetHandle(const std::filesystem::path& assetPath) const
	{
		// TODO: Just load meta file to get handle
		for (auto& asset : m_AssetRegistry)
		{
			if (assetPath.extension() == ".fbx" || 
				assetPath.extension() == ".FBX" ||
				assetPath.extension() == ".obj")
			{
				if (asset.second.type != AssetType::Model)
				continue;
			}
			if (asset.second.assetFilePath == assetPath)
				return asset.first;
		}
		return 0;
	}

	AssetType AssetManager::GetAssetType(AssetHandle handle) const
	{
		return m_AssetRegistry.at(handle).type;
	}

	const std::filesystem::path& AssetManager::GetAssetPath(AssetHandle handle) const
	{
		return m_AssetRegistry.at(handle).assetFilePath;
	}

	std::filesystem::path AssetManager::GetAssetName(AssetHandle handle)
	{
		// TODO: Use more good method
		ASSERT(handle);
		const auto& metadata = m_AssetRegistry.at(handle);
		if (metadata.assetFilePath.extension() == ".fbx" ||
			metadata.assetFilePath.extension() == ".FBX" ||
			metadata.assetFilePath.extension() == ".obj")
		{
			auto modelHandle = GetHandle(metadata.assetFilePath);
			auto model = GetAsset<Model>(modelHandle);

			if (metadata.type == AssetType::Material)
			{
				for (size_t i = 0; i < model->GetMaterials().size(); ++i)
				{
					if (model->GetMaterials()[i] == handle)
					{
						return model->GetMaterialsName()[i];
					}
				}
			}

			if (metadata.type == AssetType::Mesh)
			{
				for (size_t i = 0; i < model->GetMeshes().size(); ++i)
				{
					if (model->GetMeshes()[i] == handle)
					{
						return model->GetMeshesName()[i];
					}
				}
			}
		}
		return metadata.assetFilePath.stem();
	}

	void AssetManager::UpdateRuntimeAsset(const std::string& name, const Shared<Asset>& asset)
	{
		ASSERT(!name.empty());
		m_RuntimeAsset[name] = asset;
	}

	Shared<Asset> AssetManager::GetRuntimeAsset(const std::string& name) const
	{
		return m_RuntimeAsset.at(name);
	}

	bool AssetManager::LoadAsset(AssetHandle handle)
	{
		auto& metadata = m_AssetRegistry.at(handle);
		auto importer = AssetImporter::Create(metadata.type, m_AssetDirectory / metadata.assetFilePath);
		importer->Deserialize();
		auto asset = importer->Import();
		if (metadata.type == AssetType::Model)
			importer->Serialize(handle); // when the first time load model
		if (!asset)
		{
			LUMEN_RUNTIME_ERROR("Cannot load the file: {0}", metadata.assetFilePath.string());
			return false;
		}
		m_LoadedAssets[handle] = asset;

		return true;
	}

	Shared<Asset> AssetManager::GetAsset(AssetHandle handle)
	{
		if (m_LoadedAssets.find(handle) == m_LoadedAssets.end())
			LoadAsset(handle);

		return m_LoadedAssets.at(handle);
	}

	void AssetManager::UpdateAsset(AssetHandle handle, Shared<Asset> asset)
	{
		ASSERT(GetAssetType(handle) == asset->GetType());
		m_LoadedAssets[handle] = asset;
	}

	bool AssetManager::RenameAsset(AssetHandle handle, const std::filesystem::path& name)
	{
		if (handle == 0 || m_AssetRegistry.find(handle) == m_AssetRegistry.end())
			return false;

		auto& metadata = m_AssetRegistry.at(handle);
		metadata.assetFilePath = metadata.assetFilePath.parent_path() / name;
		if (metadata.type == AssetType::Model)
		{
			ModelImporter importer(m_AssetDirectory / metadata.assetFilePath);
			importer.Deserialize();
			auto& settings = importer.GetModelImportSettings();
			for (const auto& h : settings.Meshes)
				m_AssetRegistry.at(h).assetFilePath = metadata.assetFilePath;

			for (const auto& h : settings.Materials)
				m_AssetRegistry.at(h).assetFilePath = metadata.assetFilePath;

			for (const auto& h : settings.Animations)
				m_AssetRegistry.at(h).assetFilePath = metadata.assetFilePath;
		}

		SerializeAssetRegistry(m_AssetRegistryPath);

		if (metadata.type == AssetType::Folder)
		{
			for (auto& entry : std::filesystem::directory_iterator(m_AssetDirectory / metadata.assetFilePath))
			{
				if (entry.path().extension() == ".meta")
					continue;
				AssetHandle asset = 0;
				{
					auto data = YAML::LoadFile(entry.path().string() + ".meta");
					asset = data["UUID"].as<UUID>();
				}
				MoveAsset(asset, handle);
			}
		}

		return true;
	}

	bool AssetManager::MoveAsset(AssetHandle handle, AssetHandle to)
	{
		if (handle == 0 || m_AssetRegistry.find(handle) == m_AssetRegistry.end())
			return false;

		auto& metadata = m_AssetRegistry.at(handle);
		if (to)
		{
			ASSERT(GetAssetType(to) == AssetType::Folder);
			metadata.assetFilePath = GetAssetPath(to) / metadata.assetFilePath.filename();
		}
		else 
			metadata.assetFilePath = metadata.assetFilePath.filename();

		if (metadata.type == AssetType::Model)
		{
			ModelImporter importer(m_AssetDirectory / metadata.assetFilePath);
			importer.Deserialize();
			auto& settings = importer.GetModelImportSettings();
			for (const auto& h : settings.Meshes)
				m_AssetRegistry.at(h).assetFilePath = metadata.assetFilePath;

			for (const auto& h : settings.Materials)
				m_AssetRegistry.at(h).assetFilePath = metadata.assetFilePath;

			for (const auto& h : settings.Animations)
				m_AssetRegistry.at(h).assetFilePath = metadata.assetFilePath;
		}

		SerializeAssetRegistry(m_AssetRegistryPath);

		if (metadata.type == AssetType::Folder)
		{
			for (auto& entry : std::filesystem::directory_iterator(m_AssetDirectory / metadata.assetFilePath))
			{
				if (entry.path().extension() == ".meta")
					continue;
				AssetHandle asset = 0;
				{
					auto data = YAML::LoadFile(entry.path().string() + ".meta");
					asset = data["UUID"].as<UUID>();
				}
				MoveAsset(asset, handle);
			}
		}

		return true;
	}

	bool AssetManager::CopyAsset(AssetHandle handle, AssetHandle to)
	{
		if (handle == 0 || m_AssetRegistry.find(handle) == m_AssetRegistry.end())
			return false;

		AssetHandle newAsset;
		AssetMetadata newMetadata;
		if (to)
		{
			ASSERT(GetAssetType(to) == AssetType::Folder);
			newMetadata.assetFilePath = GetAssetPath(to) / m_AssetRegistry[handle].assetFilePath.filename();
		}
		else // Assets Dir
			newMetadata.assetFilePath = m_AssetRegistry[handle].assetFilePath.filename();
		newMetadata.type = m_AssetRegistry[handle].type;
		m_AssetRegistry[newAsset] = newMetadata;

		auto fullPath = m_AssetDirectory / newMetadata.assetFilePath;

		if (newMetadata.type == AssetType::Folder)
		{
			for (auto& entry : std::filesystem::directory_iterator(fullPath))
			{
				if (entry.path().extension() == ".meta")
					continue;
				AssetHandle asset = 0;
				{
					auto data = YAML::LoadFile(entry.path().string() + ".meta");
					asset = data["UUID"].as<UUID>();
				}
				CopyAsset(asset, newAsset);
			}
		}

		auto importer = AssetImporter::Create(newMetadata.type, fullPath);
		importer->Deserialize();
		if (m_AssetRegistry[handle].type == AssetType::Model)
		{
			auto& settings = std::dynamic_pointer_cast<ModelImporter>(importer)->GetModelImportSettings();

			newMetadata.type = AssetType::Mesh;
			for (auto& h : settings.Meshes)
			{
				h = AssetHandle();
				m_AssetRegistry[h] = newMetadata;
			}

			newMetadata.type = AssetType::Material;
			for (auto& h : settings.Materials)
			{
				h = AssetHandle();
				m_AssetRegistry[h] = newMetadata;
			}

			newMetadata.type = AssetType::Animation;
			for (auto& h : settings.Animations)
			{
				h = AssetHandle();
				m_AssetRegistry[h] = newMetadata;
			}
		}
		importer->Serialize(newAsset);

		SerializeAssetRegistry(m_AssetRegistryPath);

		return true;
	}

	bool AssetManager::DeleteAsset(AssetHandle handle)
	{
		if (handle == 0 || m_AssetRegistry.find(handle) == m_AssetRegistry.end())
			return false;

		auto& metadata = m_AssetRegistry.at(handle);
		if (metadata.type == AssetType::Folder)
		{
			for (auto& entry : std::filesystem::directory_iterator(m_AssetDirectory / metadata.assetFilePath))
			{
				if (entry.path().extension() == ".meta")
					continue;
				AssetHandle asset = 0;
				{
					auto data = YAML::LoadFile(entry.path().string() + ".meta");
					asset = data["UUID"].as<UUID>();
				}
				DeleteAsset(asset);
			}
		}

		if (metadata.type == AssetType::Model)
		{
			ModelImporter importer(m_AssetDirectory / metadata.assetFilePath);
			importer.Deserialize();
			auto& settings = importer.GetModelImportSettings();
			for (const auto& h : settings.Meshes)
			{
				m_AssetRegistry.erase(h);
				m_LoadedAssets.erase(h);
			}
			for (const auto& h : settings.Materials)
			{
				m_AssetRegistry.erase(h);
				m_LoadedAssets.erase(h);
			}
			for (const auto& h : settings.Animations)
			{
				m_AssetRegistry.erase(h);
				m_LoadedAssets.erase(h);
			}
		}

		m_AssetRegistry.erase(handle);
		m_LoadedAssets.erase(handle);
		SerializeAssetRegistry(m_AssetRegistryPath);

		return true;
	}
}