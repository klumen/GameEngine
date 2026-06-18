#pragma once

#include "runtime/function/project/Project.h"
#include "runtime/resource/data/Texture.h"

#include <filesystem>

struct ImVec4;

namespace Lumen
{
	class ContentBrowserPanel
	{
	public:
		ContentBrowserPanel();
		~ContentBrowserPanel() = default;

		void SetProject(const Shared<Project>& project);
		void SetSelectedAsset(AssetHandle handle = 0) { m_SelectedAsset = handle; }
		void OnImGui();
		void ImportAssets(const std::filesystem::path& path);

		const std::filesystem::path& GetAssetCurrentDirectory() { return m_CurrentDirectory; }

	private:
		void DrawNavigationBar();
		void DrawContextMenu();
		void DrawFileGrid();
		void ProcessPasteOperation();
		void SetOperationStatus(const std::string& text, const ImVec4& color);
;		void ShowOperationStatus();
		void DrawAsset();

	private:
		Shared<Project> m_Project;

		Shared<Texture> m_FolderIcon;
		Shared<Texture> m_FileIcon;

		std::filesystem::path m_AssetDirectory;
		std::filesystem::path m_CurrentDirectory;

		std::vector<std::filesystem::path> m_RemoveFiles;

		AssetHandle m_SelectedAsset = 0;

	};
}