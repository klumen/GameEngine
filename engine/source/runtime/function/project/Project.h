#pragma once

#include "runtime/resource/asset/Asset.h"
#include "runtime/core/Memory.h"

#include <filesystem>

namespace Lumen
{
	class Project
	{
		friend class ProjectSerializer;

	public:
		Project() = default;
		~Project() = default;

		const std::filesystem::path& GetPath() const { return m_Path; }
		void SetPath(const std::filesystem::path& projectPath) { m_Path = projectPath; }
		AssetHandle GetStartScene() const { return m_StartScene; }
		void SetStartScene(AssetHandle handle) { m_StartScene = handle; }
		std::filesystem::path GetProjectDirectory() const { return m_Path.parent_path(); }
		std::filesystem::path GetAseetDirectory() const { return GetProjectDirectory() / m_AssetDirectory; }
		std::filesystem::path GetAseetRegister() const { return GetAseetDirectory() / m_AssetRegisterName; }

	private:
		std::filesystem::path m_Path;
		AssetHandle m_StartScene{ 0 };
		std::filesystem::path m_AssetDirectory = "Assets";
		std::filesystem::path m_AssetRegisterName = "AssetRegister.meta"; // relative to asset dir

	};
}