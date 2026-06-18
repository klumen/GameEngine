#include "runtime/resource/asset/FolderImporter.h"

#include "runtime/resource/data/Folder.h"
#include "runtime/resource/asset/Serializer.h"
#include "runtime/core/Macro.h"

namespace Lumen
{
	FolderImporter::FolderImporter(const std::filesystem::path& path)
		: AssetImporter(path) { }

	Shared<Asset> FolderImporter::Import()
	{
		return MakeShared<Folder>();
	}

	bool FolderImporter::CreateAsset()
	{
		return std::filesystem::create_directories(m_AssetPath);
	}

	bool FolderImporter::Serialize(AssetHandle handle)
	{
		YAML::Emitter out;
		out << YAML::BeginMap;
		out << YAML::Key << "UUID" << YAML::Value << handle;
		out << YAML::Key << "FolderImporter" << YAML::Value;
		{
			out << YAML::BeginMap;
			out << YAML::EndMap;
		}
		out << YAML::EndMap;

		std::ofstream fout(m_AssetPath.string() + ".meta");
		ASSERT(fout.is_open());
		fout << out.c_str();

		return true;
	}

	AssetHandle FolderImporter::Deserialize()
	{
		auto data = YAML::LoadFile(m_AssetPath.string() + ".meta");
		ASSERT(data);

		AssetHandle handle = data["UUID"].as<UUID>();
		auto settings = data["FolderImporter"];
		ASSERT(settings);

		return handle;
	}
}