#include "runtime/resource/asset/DefaultImporter.h"

#include "runtime/resource/data/Default.h"
#include "runtime/resource/asset/Serializer.h"
#include "runtime/core/Macro.h"

namespace Lumen
{
	DefaultImporter::DefaultImporter(const std::filesystem::path& path)
		: AssetImporter(path) { }

	Shared<Asset> DefaultImporter::Import()
	{
		return MakeShared<Default>();
	}

	bool DefaultImporter::Serialize(AssetHandle handle)
	{
		YAML::Emitter out;
		out << YAML::BeginMap;
		out << YAML::Key << "UUID" << YAML::Value << handle;
		out << YAML::Key << "DefaultImporter" << YAML::Value;
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

	AssetHandle DefaultImporter::Deserialize()
	{
		auto data = YAML::LoadFile(m_AssetPath.string() + ".meta");
		ASSERT(data);

		AssetHandle handle = data["UUID"].as<UUID>();
		auto settings = data["DefaultImporter"];
		ASSERT(settings);

		return handle;
	}
}