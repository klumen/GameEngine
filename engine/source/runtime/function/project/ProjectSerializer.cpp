#include "runtime/function/project/ProjectSerializer.h"

#define YAML_CPP_STATIC_DEFINE
#include <yaml-cpp/yaml.h>

#include <fstream>

namespace Lumen
{
	ProjectSerializer::ProjectSerializer(Shared<Project> project)
		: m_Project(project) { }

	bool ProjectSerializer::Serialize()
	{
		YAML::Emitter out;
		{
			out << YAML::BeginMap;
			out << YAML::Key << "Project" << YAML::Value;
			{
				out << YAML::BeginMap;
				out << YAML::Key << "StartScene" << YAML::Value << m_Project->m_StartScene;
				out << YAML::Key << "AssetDirectory" << YAML::Value << m_Project->m_AssetDirectory.string();
				out << YAML::EndMap;
			}
			out << YAML::EndMap;
		}

		std::ofstream fout(m_Project->m_Path);
		if (fout.is_open())
		{
			fout << out.c_str();
			fout.close();
			return true;
		}
		else
			return false;
	}

	bool ProjectSerializer::Deserialize()
	{
		YAML::Node data = YAML::LoadFile(m_Project->m_Path.string());
		auto projectNode = data["Project"];
		if (!projectNode)
			return false;

		m_Project->m_StartScene = projectNode["StartScene"].as<uint64_t>();
		m_Project->m_AssetDirectory = projectNode["AssetDirectory"].as<std::string>();

		return true;
	}
}