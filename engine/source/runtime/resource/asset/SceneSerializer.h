#pragma once

#include "runtime/resource/data/Scene.h"

#define YAML_CPP_STATIC_DEFINE
#include <yaml-cpp/yaml.h>

#include <filesystem>

namespace Lumen
{
	class SceneSerializer
	{
	public:
		SceneSerializer(const Shared<Scene>& scene);
		~SceneSerializer() = default;

		bool Serialize(const std::filesystem::path& scenePath);
		bool Deserialize(const std::filesystem::path& scenePath);

	private:
		bool SerializeEntity(YAML::Emitter& out, Entity entity);

		Shared<Scene> m_Scene;

	};
}