#pragma once

#include "editor/panel/ContentBrowserPanel.h"
#include "editor/panel/SceneHierarchyPanel.h"
#include "editor/EditorCamera.h"

#include "runtime/Application.h"
#include "runtime/function/HID/ApplicationEvent.h"
#include "runtime/function/HID/KeyEvent.h"
#include "runtime/function/render/FrameBuffer.h"

#include <glm/glm.hpp>

namespace Lumen
{
	class Editor : public Application
	{
	public:
		static Editor& Instance()
		{
			static Editor singleton;
			return singleton;
		}

		virtual void StartUp() override;
		virtual void Run() override;
		virtual void ShutDown() override;

	private:
		void OnUpdate();

		void OnScenePlay();
		void OnSceneEdit();

		void OnEvent(Event& event);
		bool OnWindowClose(WindowCloseEvent& event);
		bool OnKeyPressed(KeyPressedEvent& event);
		bool OnWindowDrop(WindowDropEvent& event);

		void NewProject();
		bool LoadProject();
		bool LoadProject(const std::filesystem::path& projectPath);
		bool SaveProject();
		bool SaveScene();

		void OnImGui();
		void ViewportImGui();
		void ToolbarImGui();

	private:
		Editor() = default;
		virtual ~Editor() = default;
		Editor(const Editor&) = delete;
		Editor& operator=(const Editor&) = delete;

		bool m_Running = true;
		Shared<Project> m_Project;
		AssetHandle m_Scene = 0;

		Unique<ContentBrowserPanel> m_ContentBrowserPanel;
		Unique<SceneHierarchyPanel> m_SceneHierarchyPanel;

		bool m_ViewportFocused = false, m_ViewportHovered = false;
		glm::vec2 m_ViewportSize = { 0.f, 0.f };
		glm::vec2 m_ViewportBounds[2]{ {0.f, 0.f}, {0.f, 0.f} };

		enum class SceneState : uint8_t
		{
			Edit = 1, Play = 2
		};
		SceneState m_SceneState = SceneState::Edit;
		Shared<Texture> m_PlayIcon;
		Shared<Texture> m_StopIcon;

		Unique<EditorCamera> m_EditorCamera;
		int32_t m_GizmoType = -1;

	};
}