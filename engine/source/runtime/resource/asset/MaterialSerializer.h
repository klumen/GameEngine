#pragma once

#include "runtime/resource/data/Material.h"

#define YAML_CPP_STATIC_DEFINE
#include <yaml-cpp/yaml.h>

namespace Lumen
{
	class MaterialSerializer
	{
	public:
		MaterialSerializer(const Shared<Material>& material);
		~MaterialSerializer() = default;

		bool Serialize(const std::filesystem::path& path);
		bool Deserialize(const std::filesystem::path& path);

	private:
		void SerializeProperties(YAML::Emitter& out);

		Shared<Material> m_Material;

	};
}