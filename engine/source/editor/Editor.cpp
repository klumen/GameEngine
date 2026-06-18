#include "editor/Editor.h"

#include "runtime/Engine.h"
#include "runtime/function/global/GlobalContext.h"
#include "runtime/function/render/Window.h"
#include "runtime/function/render/RenderSystem.h"
#include "runtime/function/render/Time.h"
#include "runtime/function/imgui/IMGUIRenderer.h"
#include "runtime/function/project/ProjectSerializer.h"
#include "runtime/function/HID/Input.h"
#include "runtime/resource/config/ConfigManager.h"
#include "runtime/resource/asset/AssetManager.h"
#include "runtime/resource/asset/SceneSerializer.h"
#include "runtime/resource/asset/SceneImporter.h"
#include "runtime/resource/asset/TextureImporter.h"
#include "runtime/resource/asset/ModelImporter.h"
#include "runtime/resource/data/Folder.h"
#include "runtime/core/FileSystem.h"
#include "runtime/core/Log.h"
#include "runtime/core/Macro.h"

#include <glm/gtc/type_ptr.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/matrix_decompose.hpp>

#include <ImGuizmo.h>

namespace Lumen
{
	Application& Application::Instance()
	{
		return Editor::Instance();
	}

	void Editor::StartUp()
	{
		TextureImporter playIconImporter(LUMEN_CONFIG_MANAGER->GetPlayIcon());
		m_PlayIcon = std::dynamic_pointer_cast<Texture>(playIconImporter.Import());
		TextureImporter stopIconImporter(LUMEN_CONFIG_MANAGER->GetStopIcon());
		m_StopIcon = std::dynamic_pointer_cast<Texture>(stopIconImporter.Import());

		IMGUIRenderer::StartUp();
		LUMEN_WINDOW->SetEventCallback(LUMEN_BIND_EVENT_FN(OnEvent));

		m_ContentBrowserPanel = MakeUnique<ContentBrowserPanel>();
		m_SceneHierarchyPanel = MakeUnique<SceneHierarchyPanel>();

		m_EditorCamera = MakeUnique<EditorCamera>();
	}

	void Editor::Run()
	{
		Time::deltaTime = 1.f / 60.f;
		Time::lastTime = Time::GetTime();

		while (m_Running)
		{
			LUMEN_RENDER_SYSTEM->Begin();

			IMGUIRenderer::Begin();
			OnImGui();
			IMGUIRenderer::End();

			if (m_Project && m_Scene)
				OnUpdate();

			LUMEN_RENDER_SYSTEM->End();
			LUMEN_WINDOW->PollEvents();

			Time::Update();
		}
	}

	void Editor::ShutDown()
	{
		IMGUIRenderer::ShutDown();
	}


	void Editor::OnUpdate()
	{
		// Resize
		if (m_ViewportSize.x > 0.f && m_ViewportSize.y > 0.f)
		{
			LUMEN_RENDER_SYSTEM->OnSetViewport((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
			m_EditorCamera->SetViewport(m_ViewportSize.x, m_ViewportSize.y);
		}

		m_EditorCamera->OnUpdate();
		LUMEN_RENDER_SYSTEM->OnSetViewProject(
			m_EditorCamera->GetViewMatrix(), m_EditorCamera->GetProjectionMatrix(), m_EditorCamera->GetPosition());

		LUMEN_ASSET_MANAGER->GetAsset<Scene>(m_Scene)->Update();

		LUMEN_RENDER_SYSTEM->Update();
	}

	void Editor::OnScenePlay()
	{

	}

	void Editor::OnSceneEdit()
	{

	}

	void Editor::OnEvent(Event& event)
	{
		if (m_ViewportHovered)
			m_EditorCamera->OnEvent(event);

		EventDispatcher dispatcher(event);
		dispatcher.Dispatch<WindowCloseEvent>(LUMEN_BIND_EVENT_FN(OnWindowClose));
		dispatcher.Dispatch<KeyPressedEvent>(LUMEN_BIND_EVENT_FN(OnKeyPressed));
		dispatcher.Dispatch<WindowDropEvent>(LUMEN_BIND_EVENT_FN(OnWindowDrop));

		if (event.Handled)
		{
			// TODO:
		}
	}

	bool Editor::OnWindowClose(WindowCloseEvent& event)
	{
		m_Running = false;
		return true;
	}

	bool Editor::OnKeyPressed(KeyPressedEvent& event)
	{
		// Shortcuts
		if (event.IsRepeat())
			return false;

		bool control = Input::IsKeyPressed(Key::LeftControl) || Input::IsKeyPressed(Key::RightControl);
		bool shift = Input::IsKeyPressed(Key::LeftShift) || Input::IsKeyPressed(Key::RightShift);

		switch (event.GetKeyCode())
		{
		case Key::N:
		{
			if (control)
				NewProject();

			break;
		}
		case Key::L:
		{
			if (control)
				LoadProject();

			break;
		}
		case Key::S:
		{
			if (control)
				SaveProject();

			break;
		}

		// Gizmos
		case Key::Q:
		{
			if (!ImGuizmo::IsUsing())
				m_GizmoType = -1;
			break;
		}
		case Key::W:
		{
			if (!ImGuizmo::IsUsing())
				m_GizmoType = ImGuizmo::OPERATION::TRANSLATE;
			break;
		}
		case Key::E:
		{
			if (!ImGuizmo::IsUsing())
				m_GizmoType = ImGuizmo::OPERATION::ROTATE;
			break;
		}
		/*case Key::R:
		{
			if (control)
			{
				ScriptEngine::ReloadAssembly();
			}
			else
			{
				if (!ImGuizmo::IsUsing())
					m_GizmoType = ImGuizmo::OPERATION::SCALE;
			}
			break;
		}
		case Key::F:
		{
			if (m_SelectionContext.size() == 0)
				break;

			Entity selectedEntity = m_SelectionContext[0].Entity;
			m_EditorCamera.Focus(selectedEntity.Transform().Translation);
			break;
		}
		case Key::Delete:
		{
			if (Application::Get().GetImGuiLayer()->GetActiveWidgetID() == 0)
			{
				Entity selectedEntity = m_SceneHierarchyPanel.GetSelectedEntity();
				if (selectedEntity)
				{
					m_SceneHierarchyPanel.SetSelectedEntity({});
					m_ActiveScene->DestroyEntity(selectedEntity);
				}
			}
			break;
		}*/
		}

		return false;
	}

	bool Editor::OnWindowDrop(WindowDropEvent& event)
	{
		for (const auto& path : event.GetPaths())
		{
			//if (path.extension() == ".luproj")

			m_ContentBrowserPanel->ImportAssets(path);
		}

		return true;
	}

	void Editor::OnImGui()
	{
		// Note: Switch this to true to enable dockspace
		static bool dockspaceOpen = true;
		static bool opt_fullscreen_persistant = true;
		bool opt_fullscreen = opt_fullscreen_persistant;
		static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

		// We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
		// because it would be confusing to have two docking targets within each others.
		ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
		if (opt_fullscreen)
		{
			ImGuiViewport* viewport = ImGui::GetMainViewport();
			ImGui::SetNextWindowPos(viewport->Pos);
			ImGui::SetNextWindowSize(viewport->Size);
			ImGui::SetNextWindowViewport(viewport->ID);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
			window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
			window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
		}

		// When using ImGuiDockNodeFlags_PassthruCentralNode, DockSpace() will render our background and handle the pass-thru hole, so we ask Begin() to not render a background.
		if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
			window_flags |= ImGuiWindowFlags_NoBackground;

		// Important: note that we proceed even if Begin() returns false (aka window is collapsed).
		// This is because we want to keep our DockSpace() active. If a DockSpace() is inactive, 
		// all active windows docked into it will lose their parent and become undocked.
		// We cannot preserve the docking relationship between an active window and an inactive docking, otherwise 
		// any change of dockspace/settings would lead to windows being stuck in limbo and never being visible.
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		ImGui::Begin("DockSpace", &dockspaceOpen, window_flags);
		ImGui::PopStyleVar();

		if (opt_fullscreen)
			ImGui::PopStyleVar(2);

		// DockSpace
		ImGuiIO& io = ImGui::GetIO();
		ImGuiStyle& style = ImGui::GetStyle();
		float minWinSizeX = style.WindowMinSize.x;
		style.WindowMinSize.x = 370.0f;
		if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
		{
			ImGuiID dockspace_id = ImGui::GetID("DockSpace");
			ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
		}

		style.WindowMinSize.x = minWinSizeX;

		if (ImGui::BeginMenuBar())
		{
			if (ImGui::BeginMenu("File"))
			{
				if (ImGui::MenuItem("New Project", "Ctrl+N"))
					NewProject();

				if (ImGui::MenuItem("Load Project ...", "Ctrl+L"))
					LoadProject();

				if (ImGui::MenuItem("Save Project", "Ctrl+S"))
					SaveProject();
				
				ImGui::Separator();

				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Script"))
			{
				/*if (ImGui::MenuItem("Reload assembly", "Ctrl+R"))
					ScriptEngine::ReloadAssembly();*/

				ImGui::EndMenu();
			}

			ImGui::EndMenuBar();
		}

		static bool showDemoWindow = true;
		ImGui::ShowDemoWindow(&showDemoWindow);

		if (m_Project)
		{
			m_ContentBrowserPanel->OnImGui();
			m_SceneHierarchyPanel->OnImGui();
			ViewportImGui();
			ToolbarImGui();
		}

		ImGui::End();
	}

	void Editor::ViewportImGui()
	{
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 0, 0 });
		ImGui::Begin("Viewport");

		auto viewportMinRegion = ImGui::GetWindowContentRegionMin();
		auto viewportMaxRegion = ImGui::GetWindowContentRegionMax();
		auto viewportOffset = ImGui::GetWindowPos();
		m_ViewportBounds[0] = { viewportMinRegion.x + viewportOffset.x, viewportMinRegion.y + viewportOffset.y };
		m_ViewportBounds[1] = { viewportMaxRegion.x + viewportOffset.x, viewportMaxRegion.y + viewportOffset.y };

		m_ViewportFocused = ImGui::IsWindowFocused();
		m_ViewportHovered = ImGui::IsWindowHovered();

		ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
		m_ViewportSize = { viewportPanelSize.x, viewportPanelSize.y };

		uint32_t textureID = LUMEN_RENDER_SYSTEM->GetFinalImageRendererID();
		ImGui::Image(textureID, ImVec2{ m_ViewportSize.x, m_ViewportSize.y }, ImVec2{ 0, 1 }, ImVec2{ 1, 0 });

		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
			{
				ASSERT(payload->DataSize == sizeof(AssetHandle));
				AssetHandle handle = *(AssetHandle*)payload->Data;
				if (LUMEN_ASSET_MANAGER->GetAssetType(handle) == AssetType::Scene)
				{
					m_SceneHierarchyPanel->SetScene(handle);
					m_SceneHierarchyPanel->SetSelectedEntity();
				}
			}
			ImGui::EndDragDropTarget();
		}

		// Gizmos
		Entity selectedEntity = m_SceneHierarchyPanel->GetSelectedEntity();
		if (selectedEntity && m_GizmoType != -1)
		{
			ImGuizmo::SetGizmoSizeClipSpace(0.2f);

			ImGuizmo::SetOrthographic(false);
			ImGuizmo::SetDrawlist();

			ImGuizmo::SetRect(m_ViewportBounds[0].x, m_ViewportBounds[0].y, m_ViewportBounds[1].x - m_ViewportBounds[0].x, m_ViewportBounds[1].y - m_ViewportBounds[0].y);

			// Camera

			// Runtime camera from entity
			// auto cameraEntity = m_ActiveScene->GetPrimaryCameraEntity();
			// const auto& camera = cameraEntity.GetComponent<CameraComponent>().Camera;
			// const glm::mat4& cameraProjection = camera.GetProjection();
			// glm::mat4 cameraView = glm::inverse(cameraEntity.GetComponent<TransformComponent>().GetTransform());

			// Editor camera
			const glm::mat4& cameraProjection = m_EditorCamera->GetProjectionMatrix();
			glm::mat4 cameraView = m_EditorCamera->GetViewMatrix();

			// Entity transform
			auto& tc = selectedEntity.GetComponent<TransformComponent>();
			glm::mat4 transform = tc.GetTransform();

			// Snapping
			bool snap = Input::IsKeyPressed(Key::LeftControl);
			float snapValue = 0.5f; // Snap to 0.5m for translation/scale
			// Snap to 45 degrees for rotation
			if (m_GizmoType == ImGuizmo::OPERATION::ROTATE)
				snapValue = 45.0f;

			float snapValues[3] = { snapValue, snapValue, snapValue };

			ImGuizmo::Manipulate(glm::value_ptr(cameraView), glm::value_ptr(cameraProjection),
				(ImGuizmo::OPERATION)m_GizmoType, ImGuizmo::LOCAL, glm::value_ptr(transform),
				nullptr, snap ? snapValues : nullptr);

			if (ImGuizmo::IsUsing())
			{
				glm::vec3 translation, scale, skew;
				glm::quat rotation;
				glm::vec4 persp;
				glm::decompose(transform, scale, rotation, translation, skew, persp);

				glm::vec3 deltaRotation = glm::degrees(glm::eulerAngles(rotation)) - tc.rotation;
				tc.translation = translation;
				tc.rotation += deltaRotation;
				tc.scale = scale;
			}
		}

		ImGui::End();
		ImGui::PopStyleVar();
	}

	void Editor::ToolbarImGui()
	{
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
		ImGui::PushStyleVar(ImGuiStyleVar_ItemInnerSpacing, ImVec2(0, 0));
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));

		ImGui::Begin("##ToolBar", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

		float size = ImGui::GetWindowHeight() - 4.f;
		Shared<Texture> icon = m_SceneState == SceneState::Edit ? m_PlayIcon : m_StopIcon;
		ImGui::SetCursorPosX((ImGui::GetWindowContentRegionMax().x * 0.5f) - (size * 0.5f));
		if (ImGui::ImageButton("play/edit", icon->GetRendererID(), ImVec2(size, size), { 0, 1 }, { 1, 0 }))
		{
			if (m_SceneState == SceneState::Edit)
				OnScenePlay();
			else if (m_SceneState == SceneState::Play)
				OnSceneEdit();
		}

		//ImGui::SameLine();
		//Ref<Texture> icon = 

		ImGui::End();

		ImGui::PopStyleVar(2);
		ImGui::PopStyleColor();
	}

	void Editor::NewProject()
	{
		std::filesystem::path filepath = FileDialog::SelectDirectory();
		m_Project = MakeShared<Project>();
		m_Project->SetPath(filepath / (filepath.filename().string() + ".luproj"));

		std::filesystem::create_directories(m_Project->GetAseetDirectory());
		LUMEN_ASSET_MANAGER->SetAssetDirectory(m_Project->GetAseetDirectory());
		LUMEN_ASSET_MANAGER->SetAssetRegistryPath(m_Project->GetAseetRegister());

		auto dir = LUMEN_ASSET_MANAGER->CreateAsset<Folder>("Scenes");
		m_Scene = LUMEN_ASSET_MANAGER->CreateAsset<Scene>("MainScene.lumen", dir);
		m_Project->SetStartScene(m_Scene);

		ProjectSerializer serializer(m_Project);
		serializer.Serialize();

		m_ContentBrowserPanel->SetProject(m_Project);
		m_ContentBrowserPanel->SetSelectedAsset();
		m_SceneHierarchyPanel->SetScene(m_Scene);
		m_SceneHierarchyPanel->SetSelectedEntity();
	}

	bool Editor::LoadProject()
	{
		std::filesystem::path filepath = FileDialog::LoadFile("Lumen Project (*.luproj)\0*.luproj\0");
		if (filepath.empty())
			return false;

		return LoadProject(filepath);
	}

	bool Editor::LoadProject(const std::filesystem::path& projectPath)
	{
		Shared<Project> project = MakeShared<Project>();
		project->SetPath(projectPath);

		ProjectSerializer serializer(project);
		if (serializer.Deserialize())
			if (LUMEN_ASSET_MANAGER->DeserializeAssetRegistry(project->GetAseetRegister()))
				m_Project = std::move(project);
			else return false;
		else return false;

		LUMEN_ASSET_MANAGER->SetAssetDirectory(m_Project->GetAseetDirectory());
		LUMEN_ASSET_MANAGER->SetAssetRegistryPath(m_Project->GetAseetRegister());
		m_Scene = m_Project->GetStartScene();

		//ScriptEngine::Init();

		m_ContentBrowserPanel->SetProject(m_Project);
		m_ContentBrowserPanel->SetSelectedAsset();
		m_SceneHierarchyPanel->SetScene(m_Scene);
		m_SceneHierarchyPanel->SetSelectedEntity();

		return true;
	}

	bool Editor::SaveProject()
	{
		ASSERT(!m_Project->GetPath().empty());

		LUMEN_ASSET_MANAGER->SerializeAssetRegistry(m_Project->GetAseetRegister());
		ProjectSerializer serializer(m_Project);
		SaveScene();
		return serializer.Serialize();
	}

	bool Editor::SaveScene()
	{
		auto scene = LUMEN_ASSET_MANAGER->GetAsset<Scene>(m_Scene);
		ASSERT(scene);
		SceneSerializer sceneSerializer(scene);
		return sceneSerializer.Serialize(m_Project->GetAseetDirectory()
			/ LUMEN_ASSET_MANAGER->GetAssetPath(m_Scene));
	}
}