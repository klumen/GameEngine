#include "editor/panel/ContentBrowserPanel.h"

#include "runtime/function/global/GlobalContext.h"
#include "runtime/function/imgui/IMGUIRenderer.h"
#include "runtime/resource/asset/AssetManager.h"
#include "runtime/resource/asset/TextureImporter.h"
#include "runtime/resource/asset/SceneImporter.h"
#include "runtime/resource/asset/ModelImporter.h"
#include "runtime/resource/asset/MaterialImporter.h"
#include "runtime/resource/asset/MaterialSerializer.h"
#include "runtime/resource/data/Model.h"
#include "runtime/resource/config/ConfigManager.h"
#include "runtime/resource/data/Folder.h"
#include "runtime/core/Macro.h"

#include <glm/gtc/type_ptr.hpp>

namespace Lumen
{
	struct ClipboardItem
	{
		std::filesystem::path path;
		bool isCutOperation = false;
	};
	static ClipboardItem s_Clipboard;
	
	static bool s_ShowOperationStatus = false;
	static std::string s_OperationStatusText;
	static ImVec4 s_OperationStatusColor = ImVec4(1.f, 0.f, 0.f, 1.f);

	static Shared<AssetImporter> s_Importer;
	static AssetHandle s_NowHandle = 0;

	ContentBrowserPanel::ContentBrowserPanel()
	{
		TextureImporter folderIconImporter(LUMEN_CONFIG_MANAGER->GetFolderIcon());
		m_FolderIcon = std::dynamic_pointer_cast<Texture>(folderIconImporter.Import());
		TextureImporter fileIconImporter(LUMEN_CONFIG_MANAGER->GetFileIcon());
		m_FileIcon = std::dynamic_pointer_cast<Texture>(fileIconImporter.Import());
	}

	void ContentBrowserPanel::SetProject(const Shared<Project>& project)
	{
		ASSERT(project);
		m_Project = project;
		m_CurrentDirectory = m_AssetDirectory = project->GetAseetDirectory();
	}

	void ContentBrowserPanel::ImportAssets(const std::filesystem::path& path)
	{
		std::filesystem::path target = m_CurrentDirectory / path.filename();
		std::filesystem::copy(path, target,
			std::filesystem::copy_options::recursive |
			std::filesystem::copy_options::overwrite_existing);

		std::filesystem::path relative = std::filesystem::relative(target, m_AssetDirectory);
		LUMEN_ASSET_MANAGER->ImportAssets(relative);
	}

	void ContentBrowserPanel::OnImGui()
	{
		ASSERT(m_Project);

		ImGui::Begin("Content Browser");

		/*if (ImGui::IsMouseDown(0) && ImGui::IsWindowHovered())
			m_SelectedAsset = 0;*/

		DrawNavigationBar();

		DrawContextMenu();

		DrawFileGrid();

		ShowOperationStatus();

		ImGui::End();

		for (const auto& file : m_RemoveFiles)
		{
			auto relativePath = std::filesystem::relative(file, m_AssetDirectory);
			auto handle = LUMEN_ASSET_MANAGER->GetHandle(relativePath);
			LUMEN_ASSET_MANAGER->DeleteAsset(handle);
			if (handle == m_SelectedAsset)
				m_SelectedAsset = 0;

			std::filesystem::remove_all(file);
			std::filesystem::remove_all(file.string() + ".meta");

			SetOperationStatus("Delete Succeed!", ImVec4(0.f, 1.f, 0.f, 1.f));
		}
		m_RemoveFiles.clear();

		DrawAsset();
	}

	void ContentBrowserPanel::DrawNavigationBar()
	{
		if (ImGui::Button("<-"))
			if (m_CurrentDirectory != m_AssetDirectory)
				m_CurrentDirectory = m_CurrentDirectory.parent_path();
		ImGui::SameLine();

		std::filesystem::path relative = std::filesystem::relative(m_CurrentDirectory, m_AssetDirectory);
		if (relative == ".")
			ImGui::TextUnformatted("Assets");
		else
			ImGui::TextUnformatted(("Assets" / relative).string().c_str());
	}

	void ContentBrowserPanel::DrawContextMenu()
	{
		if (ImGui::BeginPopupContextWindow("ContentBrowserContextMenu"))
		{
			if (ImGui::MenuItem("Create Folder"))
			{
				if (!std::filesystem::exists(m_CurrentDirectory / std::filesystem::path("New Floder")))
				{
					auto dir = LUMEN_ASSET_MANAGER->GetHandle(
						std::filesystem::relative(m_CurrentDirectory, m_AssetDirectory));
					LUMEN_ASSET_MANAGER->CreateAsset<Folder>("New Floder", dir);

					SetOperationStatus("Create Folder Succeed!", ImVec4(0.f, 1.f, 0.f, 1.f));
				}
				else
				{
					SetOperationStatus("A 'New Floder' exists!", ImVec4(1.f, 0.f, 0.f, 1.f));
				}
			}

			if (ImGui::MenuItem("Create Scene"))
			{
				if (!std::filesystem::exists(m_CurrentDirectory / std::filesystem::path("New Scene.lumen")))
				{
					auto dir = LUMEN_ASSET_MANAGER->GetHandle(
						std::filesystem::relative(m_CurrentDirectory, m_AssetDirectory));
					LUMEN_ASSET_MANAGER->CreateAsset<Scene>("New Scene.lumen", dir);

					SetOperationStatus("Create Scene Succeed!", ImVec4(0.f, 1.f, 0.f, 1.f));
				}
				else
				{
					SetOperationStatus("A 'New Scene' exists!", ImVec4(1.f, 0.f, 0.f, 1.f));
				}
			}

			if (ImGui::MenuItem("Create Material"))
			{
				if (!std::filesystem::exists(m_CurrentDirectory / std::filesystem::path("New Material.mat")))
				{
					auto dir = LUMEN_ASSET_MANAGER->GetHandle(
						std::filesystem::relative(m_CurrentDirectory, m_AssetDirectory));
					LUMEN_ASSET_MANAGER->CreateAsset<Material>("New Material.mat", dir);

					SetOperationStatus("Create Material Succeed!", ImVec4(0.f, 1.f, 0.f, 1.f));
				}
				else
				{
					SetOperationStatus("A 'New Material' exists!", ImVec4(1.f, 0.f, 0.f, 1.f));
				}
			}

			ImGui::Separator();

			if (ImGui::MenuItem("Paste"/*, "Ctrl+V"*/) && !s_Clipboard.path.empty())
				ProcessPasteOperation();

			ImGui::EndPopup();
		}
	}

	void ContentBrowserPanel::DrawFileGrid()
	{
		static float padding = 16.0f;
		static float thumbnailSize = 128.0f;
		const float cellSize = thumbnailSize + padding;

		const float panelWidth = ImGui::GetContentRegionAvail().x;
		int columnCount = static_cast<int>(panelWidth / cellSize);
		columnCount = columnCount < 1 ? 1 : columnCount;

		ImGui::Columns(columnCount, 0, false);

		for (auto& directoryEntry : std::filesystem::directory_iterator(m_CurrentDirectory))
		{
			const auto& filePath = directoryEntry.path();
			if (filePath.extension() == ".meta")
				continue;
			const auto& fileName = filePath.filename();

			ImGui::PushID(filePath.string().c_str());

			Shared<Texture> icon = directoryEntry.is_directory() ? m_FolderIcon : m_FileIcon;
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.f, 0.f, 0.f, 0.f));
			if (s_Clipboard.isCutOperation && s_Clipboard.path == filePath)
				ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.5f);

			ImGui::ImageButton("##Icon", (ImTextureID)icon->GetRendererID(), 
				{ thumbnailSize, thumbnailSize }, { 0, 1 }, { 1, 0 });

			ImGui::PopStyleColor();
			if (s_Clipboard.isCutOperation && s_Clipboard.path == filePath)
				ImGui::PopStyleVar();

			if (ImGui::BeginDragDropSource())
			{
				auto relativePath = std::filesystem::relative(filePath, m_AssetDirectory);
				AssetHandle handle = LUMEN_ASSET_MANAGER->GetHandle(relativePath);
				ImGui::SetDragDropPayload("CONTENT_BROWSER_ITEM", &handle, sizeof(AssetHandle));
				ImGui::Text("%s", fileName.string().c_str());

				ImGui::EndDragDropSource();
			}

			if (directoryEntry.is_directory() && ImGui::BeginDragDropTarget())
			{
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
				{
					ASSERT(payload->DataSize == sizeof(AssetHandle));
					const AssetHandle handle = *(const AssetHandle*)(payload->Data);
					const auto source = (m_AssetDirectory / LUMEN_ASSET_MANAGER->GetAssetPath(handle)).generic_string();
					const auto target = (filePath / LUMEN_ASSET_MANAGER->GetAssetPath(handle).filename()).generic_string();

					if (source == target || target.find(source + "/") == 0)
					{
						SetOperationStatus("The target directory is a subdirectory of the source!", ImVec4(1.f, 0.f, 0.f, 1.f));
					}
					else if (std::filesystem::exists(target))
					{
						SetOperationStatus("The target directory exists!", ImVec4(1.f, 0.f, 0.f, 1.f));
					}
					else
					{
						std::filesystem::rename(source, target);
						std::filesystem::rename(source + ".meta", target + ".meta");
						
						auto handle = LUMEN_ASSET_MANAGER->GetHandle(std::filesystem::relative(source, m_AssetDirectory));
						auto to = LUMEN_ASSET_MANAGER->GetHandle(std::filesystem::relative(filePath, m_AssetDirectory));
						LUMEN_ASSET_MANAGER->MoveAsset(handle, to);

						SetOperationStatus("Move Succeed!", ImVec4(0.f, 1.f, 0.f, 1.f));
					}
				}

				ImGui::EndDragDropTarget();
			}

			if (ImGui::BeginPopupContextItem())
			{
				if (ImGui::MenuItem("Copy"/*, "Ctrl+C"*/))
				{
					s_Clipboard.path = filePath;
					s_Clipboard.isCutOperation = false;
				}

				if (ImGui::MenuItem("Cut"/*, "Ctrl+X"*/))
				{
					s_Clipboard.path = filePath;
					s_Clipboard.isCutOperation = true;
				}

				if (ImGui::MenuItem("Delete"/*, "Ctrl+D"*/))
				{
					m_RemoveFiles.emplace_back(directoryEntry);
				}

				ImGui::EndPopup();
			}

			if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0))
				if (directoryEntry.is_directory())
					m_CurrentDirectory /= fileName;

			if (ImGui::IsItemClicked())
			{
				//auto relativePath = std::filesystem::relative(filePath, m_AssetDirectory);
				//m_SelectedAsset = LUMEN_ASSET_MANAGER->GetHandle(relativePath);
			}

			ImGui::TextWrapped(fileName.string().c_str());

			ImGui::NextColumn();

			if (fileName.extension() == ".fbx" || fileName.extension() == ".FBX" || fileName.extension() == ".obj")
			{
				auto relativePath = std::filesystem::relative(filePath, m_AssetDirectory);
				auto model = LUMEN_ASSET_MANAGER->GetAsset<Model>(LUMEN_ASSET_MANAGER->GetHandle(relativePath));
				
				for (size_t i = 0; i < model->GetMeshes().size(); ++i)
				{
					const auto name = fileName.string() + "/" + model->GetMeshesName()[i] + ".mesh";
					ImGui::PushID(name.c_str());

					ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.f, 0.f, 0.f, 0.f));
					ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.5f);
					ImGui::ImageButton("##Icon", (ImTextureID)m_FileIcon->GetRendererID(),
						{ thumbnailSize, thumbnailSize }, { 0, 1 }, { 1, 0 });
					ImGui::PopStyleColor();
					ImGui::PopStyleVar();

					if (ImGui::BeginDragDropSource())
					{
						ImGui::SetDragDropPayload("CONTENT_BROWSER_ITEM", &model->GetMeshes()[i], sizeof(AssetHandle));
						ImGui::Text("%s", name.c_str());

						ImGui::EndDragDropSource();
					}

					ImGui::TextWrapped(name.c_str());

					ImGui::NextColumn();
					ImGui::PopID();
				}
				
				for (size_t i = 0; i < model->GetMaterials().size(); ++i)
				{
					const auto name = fileName.string() + "/" + model->GetMaterialsName()[i] + ".mat";
					ImGui::PushID(name.c_str());

					ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.f, 0.f, 0.f, 0.f));
					ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.5f);
					ImGui::ImageButton("##Icon", (ImTextureID)m_FileIcon->GetRendererID(),
						{ thumbnailSize, thumbnailSize }, { 0, 1 }, { 1, 0 });
					ImGui::PopStyleColor();
					ImGui::PopStyleVar();

					if (ImGui::BeginDragDropSource())
					{
						ImGui::SetDragDropPayload("CONTENT_BROWSER_ITEM", &model->GetMaterials()[i], sizeof(AssetHandle));
						ImGui::Text("%s", name.c_str());

						ImGui::EndDragDropSource();
					}

					ImGui::TextWrapped(name.c_str());

					ImGui::NextColumn();
					ImGui::PopID();
				}
			}

			ImGui::PopID();
		}

		ImGui::Columns(1);

		ImGui::SliderFloat("Thumbnail Size", &thumbnailSize, 16, 512);
		ImGui::SliderFloat("Padding", &padding, 0, 32);
	}

	void ContentBrowserPanel::ProcessPasteOperation()
	{
		const auto source = s_Clipboard.path.generic_string();
		const auto target = (m_CurrentDirectory / s_Clipboard.path.filename()).generic_string();
		if (source == target || target.find(source + "/") == 0)
		{
			SetOperationStatus("The target directory is a subdirectory of the source!", ImVec4(1.f, 0.f, 0.f, 1.f));
			
			s_Clipboard.path.clear();

			return;
		}

		if (std::filesystem::exists(target))
		{
			SetOperationStatus("The target directory exists!", ImVec4(1.f, 0.f, 0.f, 1.f));

			s_Clipboard.path.clear();

			return;
		}

		auto handle = LUMEN_ASSET_MANAGER->GetHandle(std::filesystem::relative(s_Clipboard.path, m_AssetDirectory));
		auto to = LUMEN_ASSET_MANAGER->GetHandle(std::filesystem::relative(m_CurrentDirectory, m_AssetDirectory));

		if (s_Clipboard.isCutOperation)
		{
			std::filesystem::rename(source, target);
			std::filesystem::rename(source + ".meta", target + ".meta");
			
			LUMEN_ASSET_MANAGER->MoveAsset(handle, to);
		}
		else
		{
			std::filesystem::copy(source, target, std::filesystem::copy_options::recursive);
			std::filesystem::copy(source + ".meta", target + ".meta", std::filesystem::copy_options::recursive);

			LUMEN_ASSET_MANAGER->CopyAsset(handle, to);
		}

		SetOperationStatus("Paste Operation Succeed!", ImVec4(0.f, 1.f, 0.f, 1.f));

		s_Clipboard.path.clear();
	}

	void ContentBrowserPanel::SetOperationStatus(const std::string& text, const ImVec4& color)
	{
		s_ShowOperationStatus = true;
		s_OperationStatusText = text;
		s_OperationStatusColor = color;
	}

	void ContentBrowserPanel::ShowOperationStatus()
	{
		if (s_ShowOperationStatus)
		{
			ImGui::OpenPopup("##Operation Status");
			s_ShowOperationStatus = false;
		}

		if (ImGui::BeginPopup("##Operation Status"))
		{
			ImGui::PushStyleColor(ImGuiCol_Text, s_OperationStatusColor);
			ImGui::Text("%s", s_OperationStatusText.c_str());
			ImGui::PopStyleColor();

			static float displayTime = 2.f;
			static float timer = 0.f;
			timer += ImGui::GetIO().DeltaTime;

			if (timer >= displayTime)
			{
				timer = 0.f;
				ImGui::CloseCurrentPopup();
			}

			if (ImGui::Button("Cancel"))
			{
				timer = 0.f;
				ImGui::CloseCurrentPopup();
			}

			ImGui::EndPopup();
		}
	}

	void ContentBrowserPanel::DrawAsset()
	{
		ImGui::Begin("Asset");

		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
			{
				ASSERT(payload->DataSize == sizeof(AssetHandle));
				AssetHandle handle = *(AssetHandle*)payload->Data;
				m_SelectedAsset = handle;
			}
			ImGui::EndDragDropTarget();
		}

		if (!m_SelectedAsset)
		{
			ImGui::End();
			return;
		}

		auto& path = LUMEN_ASSET_MANAGER->GetAssetPath(m_SelectedAsset);
		auto name = path.stem().string();
		auto extension = path.extension().string();

		static char buf[256];
		strncpy_s(buf, sizeof(buf), name.c_str(), sizeof(buf));
		if (ImGui::InputText("##RenameInput", buf, IM_ARRAYSIZE(buf), ImGuiInputTextFlags_EnterReturnsTrue))
		{
			if (strlen(buf) > 0 && buf != name) 
			{
				auto source = (m_AssetDirectory / path.parent_path() / (name + extension)).generic_string();
				auto target = (m_AssetDirectory / path.parent_path() / (buf + extension)).generic_string();
				std::filesystem::rename(source, target);
				std::filesystem::rename(source + ".meta", target + ".meta");

				LUMEN_ASSET_MANAGER->RenameAsset(m_SelectedAsset, buf + extension);
			}
		}

		auto type = LUMEN_ASSET_MANAGER->GetAssetType(m_SelectedAsset);
		if (type == AssetType::Texture || type == AssetType::Model)
		{
			if (s_NowHandle != m_SelectedAsset)
			{
				s_NowHandle = m_SelectedAsset;
				s_Importer = AssetImporter::Create(type, m_AssetDirectory / path);

				s_Importer->Deserialize();
			}

			switch (type)
			{
			case AssetType::Texture:
			{
				auto textureImporter = std::dynamic_pointer_cast<TextureImporter>(s_Importer);
				auto& settings = textureImporter->GetTextureImportSettings();
				IMGUIRenderer::BeginPropertyGrid();

				constexpr std::array<const char*, 6> shapeNames =
				{ "None", "Texture2D", "Texture2DArray", "Cubemap", "CubemapArray", "Texture3D" };
				//IMGUIRenderer::Image(s_NowHandle, { 64, 64 });
				IMGUIRenderer::PropertyDropdown("Shape", shapeNames.data(), static_cast<int32_t>(shapeNames.size()), (int32_t*)(&settings.Shape));
				IMGUIRenderer::Property("sRGB", settings.sRGB);
				IMGUIRenderer::Property("Generate MipMaps", settings.GenerateMipMaps);

				IMGUIRenderer::EndPropertyGrid();
				break;
			}
			case AssetType::Model:
			{
				auto modelImporter = std::dynamic_pointer_cast<ModelImporter>(s_Importer);
				break;
			}
			}

			if (ImGui::Button("Enter"))
			{
				LUMEN_ASSET_MANAGER->UpdateAsset(m_SelectedAsset, s_Importer->Import());
				s_Importer->Serialize(m_SelectedAsset);
			}
		}
		else if (type == AssetType::Material)
		{
			switch (type)
			{
			case Lumen::AssetType::Material:
			{
				auto mat = LUMEN_ASSET_MANAGER->GetAsset<Material>(m_SelectedAsset);
				IMGUIRenderer::BeginPropertyGrid();
				
				constexpr std::array<const char*, 3> shaderNames = 
				{ "None", "PBRStaticShader", "SkyboxShader" };
				static int32_t selected = 0;
				if (shaderNames[selected] != mat->GetShader())
					for (size_t i = 0; i < shaderNames.size(); ++i)
						if (mat->GetShader() == shaderNames[i])
						{
							selected = static_cast<int32_t>(i);
							break;
						}

				if (IMGUIRenderer::PropertyDropdown("Shader", shaderNames.data(), static_cast<int32_t>(shaderNames.size()), &selected))
				{
					mat->SetShader(shaderNames[selected]);
					if (selected == 1)
					{
						mat->Set("u_Material.Color", glm::vec4(0.f, 0.f, 0.f, 1.f));
						mat->Set("u_Material.Metalness", 0.5f);
						mat->Set("u_Material.Roughness", 0.5f);
						mat->Set("u_AlbedoTexture", AssetHandle(0));
						mat->Set("u_Material.UseAlbedoTexture", false);
						mat->Set("u_MetalnessTexture", AssetHandle(0));
						mat->Set("u_Material.UseMetalnessTexture", false);
						mat->Set("u_RoughnessTexture", AssetHandle(0));
						mat->Set("u_Material.UseRoughnessTexture", false);
						mat->Set("u_NormalTexture", AssetHandle(0));
						mat->Set("u_Material.UseNormalTexture", false);
						mat->Set("u_HeightTexture", AssetHandle(0));
						mat->Set("u_Material.UseHeightTexture", false);
						mat->Set("u_OcclusionTexture", AssetHandle(0));
						mat->Set("u_Material.UseOcclusionTexture", false);
					}
					else if (selected == 2)
					{
						mat->Set("u_SkyboxMap", AssetHandle(0));
						mat->Set("u_Material.UseSkybox", false);
						mat->Set("u_Material.TintColor", glm::vec4(0.5f));
						mat->Set("u_Material.Exposure", 1.f);
						mat->Set("u_Material.Rotation", 0.f);
					}

					mat->SetModified(true);
				}

				if (selected == 1)
				{
					IMGUIRenderer::PropertyColor("Base Color", mat->GetVector4("u_Material.Color"));
					if (IMGUIRenderer::PropertyAssetReference("AlbedoTexture", mat->GetTexture("u_AlbedoTexture"), AssetType::Texture))
						if (mat->GetTexture("u_AlbedoTexture"))
							mat->Set("u_Material.UseAlbedoTexture", true);
						else
							mat->Set("u_Material.UseAlbedoTexture", false);

					/*if (mat->GetBool("u_Material.UseAlbedoTexture"))
					{
						IMGUIRenderer::Image(mat->GetTexture("u_AlbedoTexture"), { 64, 64 });
					}*/

					if (IMGUIRenderer::PropertyAssetReference("MetalnessTexture", mat->GetTexture("u_MetalnessTexture"), AssetType::Texture))
						if (mat->GetTexture("u_MetalnessTexture"))
							mat->Set("u_Material.UseMetalnessTexture", true);
						else
							mat->Set("u_Material.UseMetalnessTexture", false);
					
					IMGUIRenderer::PropertySlider("Metalness", mat->GetFloat("u_Material.Metalness"), 0.f, 1.f);
					
					if (IMGUIRenderer::PropertyAssetReference("RoughnessTexture", mat->GetTexture("u_RoughnessTexture"), AssetType::Texture))
						if (mat->GetTexture("u_RoughnessTexture"))
							mat->Set("u_Material.UseRoughnessTexture", true);
						else
							mat->Set("u_Material.UseRoughnessTexture", false);

					IMGUIRenderer::PropertySlider("Roughness", mat->GetFloat("u_Material.Roughness"), 0.f, 1.f);
					
					if (IMGUIRenderer::PropertyAssetReference("NormalTexture", mat->GetTexture("u_NormalTexture"), AssetType::Texture))
						if (mat->GetTexture("u_NormalTexture"))
							mat->Set("u_Material.UseNormalTexture", true);
						else
							mat->Set("u_Material.UseNormalTexture", false);

					if (IMGUIRenderer::PropertyAssetReference("HeightTexture", mat->GetTexture("u_HeightTexture"), AssetType::Texture))
						if (mat->GetTexture("u_HeightTexture"))
							mat->Set("u_Material.UseHeightTexture", true);
						else
							mat->Set("u_Material.UseHeightTexture", false);

					if (IMGUIRenderer::PropertyAssetReference("OcclusionTexture", mat->GetTexture("u_OcclusionTexture"), AssetType::Texture))
						if (mat->GetTexture("u_OcclusionTexture"))
							mat->Set("u_Material.UseOcclusionTexture", true);
						else
							mat->Set("u_Material.UseOcclusionTexture", false);
				}
				else if (selected == 2)
				{
					if (IMGUIRenderer::PropertyColor("Tint Color", mat->GetVector4("u_Material.TintColor")))
						mat->SetModified(true);

					if (IMGUIRenderer::PropertySlider("Exposure", mat->GetFloat("u_Material.Exposure"), 0.f, 10.f))
						mat->SetModified(true);

					if (IMGUIRenderer::PropertySlider("Rotation", mat->GetFloat("u_Material.Rotation"), 0.f, 360.f))
						mat->SetModified(true);

					if (IMGUIRenderer::PropertyAssetReference("Cubemap (HDR)", mat->GetTexture("u_SkyboxMap"), AssetType::Texture))
					{
						mat->SetModified(true);
						if (mat->GetTexture("u_SkyboxMap"))
							mat->Set("u_Material.UseSkybox", true);
						else
							mat->Set("u_Material.UseSkybox", false);
					}
				}

				IMGUIRenderer::EndPropertyGrid();

				if (extension == ".mat")
				{
					if (ImGui::Button("Save"))
					{
						MaterialSerializer serializer(mat);
						serializer.Serialize(m_AssetDirectory / path);
					}
				}

				break;
			}
			case Lumen::AssetType::Animation:
			{
				break;
			}
			}
		}

		ImGui::End();
	}
}