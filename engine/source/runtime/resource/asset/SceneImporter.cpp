#include "runtime/resource/asset/SceneImporter.h"

#include "runtime/resource/asset/SceneSerializer.h"
#include "runtime/resource/asset/Serializer.h"
#include "runtime/core/Macro.h"

namespace Lumen
{
    SceneImporter::SceneImporter(const std::filesystem::path& scenePath)
        : AssetImporter(scenePath) { }

    Shared<Asset> SceneImporter::Import()
    {
        Shared<Scene> scene = MakeShared<Scene>();
        SceneSerializer serializer(scene);
        serializer.Deserialize(m_AssetPath);

        return scene;
    }

    bool SceneImporter::CreateAsset()
    {
        Shared<Scene> scene = MakeShared<Scene>();
        SceneSerializer serializer(scene);
        return serializer.Serialize(m_AssetPath);
    }

    bool SceneImporter::Serialize(AssetHandle handle)
    {
        YAML::Emitter out;
        out << YAML::BeginMap;
        out << YAML::Key << "UUID" << YAML::Value << handle;
        out << YAML::Key << "SceneImporter" << YAML::Value;
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

    AssetHandle SceneImporter::Deserialize()
    {
        auto data = YAML::LoadFile(m_AssetPath.string() + ".meta");
        ASSERT(data);

        AssetHandle handle = data["UUID"].as<UUID>();
        auto settings = data["SceneImporter"];
        ASSERT(settings);

        return handle;
    }
}