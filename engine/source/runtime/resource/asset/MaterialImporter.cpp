#include "runtime/resource/asset/MaterialImporter.h"

#include "runtime/resource/asset/Serializer.h"
#include "runtime/resource/asset/MaterialSerializer.h"
#include "runtime/core/Macro.h"

namespace Lumen
{
	MaterialImporter::MaterialImporter(const std::filesystem::path& path)
		: AssetImporter(path) {}

	Shared<Asset> MaterialImporter::Import()
	{
		Shared<Material> material = MakeShared<Material>();
		MaterialSerializer serializer(material);
		serializer.Deserialize(m_AssetPath);
		
		return material;
	}

	bool MaterialImporter::CreateAsset()
	{
		Shared<Material> material = MakeShared<Material>();
		MaterialSerializer serializer(material);

		return serializer.Serialize(m_AssetPath);
	}

	bool MaterialImporter::Serialize(AssetHandle handle)
	{
		YAML::Emitter out;
		out << YAML::BeginMap;
		out << YAML::Key << "UUID" << YAML::Value << handle;
		out << YAML::Key << "MaterialImporter" << YAML::Value;
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

	AssetHandle MaterialImporter::Deserialize()
	{
		auto data = YAML::LoadFile(m_AssetPath.string() + ".meta");
		ASSERT(data);

		AssetHandle handle = data["UUID"].as<UUID>();
		auto settings = data["MaterialImporter"];
		ASSERT(settings);

		return handle;
	}
}