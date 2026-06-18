#pragma once

#include "runtime/resource/data/Scene.h"
#include "runtime/resource/data/Entity.h"

namespace Lumen
{
	class SceneHierarchyPanel
	{
	public:
		SceneHierarchyPanel() = default;
		~SceneHierarchyPanel() = default;

		void SetScene(AssetHandle scene) { m_Scene = scene; };
		void SetSelectedEntity(Entity entity = {}) { m_SelectedEntity = entity; }
		Entity GetSelectedEntity() { return m_SelectedEntity; }

		void OnImGui();

	private:
		void DrawEntityNode(Entity entity);
		void DrawComponents(Entity entity);
		void DrawLighting(Lighting& lighting);

	private:
		AssetHandle m_Scene = 0;
		Entity m_SelectedEntity;

	};
}