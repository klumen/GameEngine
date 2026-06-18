#include "editor/panel/SceneHierarchyPanel.h"

#include "runtime/function/global/GlobalContext.h"
#include "runtime/resource/asset/AssetManager.h"
#include "runtime/resource/data/Model.h"
#include "runtime/function/imgui/IMGUIRenderer.h"

#include <imgui_internal.h>
#include <glm/gtc/type_ptr.hpp>

namespace Lumen
{
	void SceneHierarchyPanel::OnImGui()
	{
		ImGui::Begin("Scene Hierarchy");

		auto scene = LUMEN_ASSET_MANAGER->GetAsset<Scene>(m_Scene);
		if (scene)
		{
			scene->GetRegistry().view<entt::entity>().each([&](auto entityID) {
				Entity entity{ entityID , scene };
				DrawEntityNode(entity);
				});

			if (ImGui::BeginDragDropTarget())
			{
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
				{
					ASSERT(payload->DataSize == sizeof(AssetHandle));
					const AssetHandle handle = *(const AssetHandle*)(payload->Data);

					if (LUMEN_ASSET_MANAGER->GetAssetType(handle) == AssetType::Model)
					{
						auto model = LUMEN_ASSET_MANAGER->GetAsset<Model>(handle);
						for (size_t i = 0; i < model->GetMeshes().size(); ++i)
						{
							const auto& mesh = model->GetMeshes()[i];
							auto entity = scene->CreateEntity(LUMEN_ASSET_MANAGER->GetAssetPath(handle).stem().string());
							auto& meshRenderer = entity.AddComponent<MeshRendererComponent>();
							meshRenderer.Mesh = mesh;
							for (const auto& mat : model->GetSubMaterials()[i])
							{
								meshRenderer.Materials.emplace_back(mat);
							}
						}
					}
				}

				ImGui::EndDragDropTarget();
			}

			if (ImGui::IsMouseDown(0) && ImGui::IsWindowHovered())
				m_SelectedEntity = {};

			if (ImGui::BeginPopupContextWindow("Scene Option", 1 | ImGuiPopupFlags_NoOpenOverItems))
			{
				if (ImGui::MenuItem("Create New Entity"))
					scene->CreateEntity("New Entity");

				ImGui::EndPopup();
			}
		}
		ImGui::End();

		DrawComponents(m_SelectedEntity);

		DrawLighting(scene->GetLighting());
	}

	void SceneHierarchyPanel::DrawEntityNode(Entity entity)
	{
		if (!entity)
			return;
		ImGui::PushID((uint32_t)entity);

		auto& name = entity.GetComponent<BaseComponent>().name;

		ImGuiTreeNodeFlags flags = (m_SelectedEntity == entity) ? ImGuiTreeNodeFlags_Selected : 0;
		flags |= ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_OpenOnArrow;
		bool opened = ImGui::TreeNodeEx((void*)(uint64_t)(uint32_t)entity, flags, name.c_str());
		if (ImGui::IsItemClicked())
			m_SelectedEntity = entity;

		bool entityDeleted = false;
		if (ImGui::BeginPopupContextWindow("Entity Option"))
		{
			if (ImGui::MenuItem("Delete Entity"))
				entityDeleted = true;

			ImGui::EndPopup();
		}

		if (opened)
		{
			ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;

			ImGui::TreePop();
		}

		ImGui::PopID();

		if (entityDeleted)
		{
			auto scene = LUMEN_ASSET_MANAGER->GetAsset<Scene>(m_Scene);
			if (scene)
			{
				scene->DestroyEntity(entity);
				if (m_SelectedEntity == entity)
					m_SelectedEntity = {};
			}
		}
	}

	static void DrawVec3Control(const std::string& label, glm::vec3& values, float resetValue = 0.0f, float columnWidth = 150.0f)
	{
		ImGuiIO& io = ImGui::GetIO();
		auto boldFont = io.Fonts->Fonts[0];

		ImGui::PushID(label.c_str());

		ImGui::Text(label.c_str());
		ImGui::NextColumn();

		ImGui::PushMultiItemsWidths(3, ImGui::CalcItemWidth());
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 0, 0 });

		float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
		ImVec2 buttonSize = { lineHeight + 3.0f, lineHeight };

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.9f, 0.2f, 0.2f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
		ImGui::PushFont(boldFont);
		if (ImGui::Button("X", buttonSize))
			values.x = resetValue;
		ImGui::PopFont();
		ImGui::PopStyleColor(3);

		ImGui::SameLine();
		ImGui::DragFloat("##X", &values.x, 0.1f, 0.0f, 0.0f, "%.2f");
		ImGui::PopItemWidth();
		ImGui::SameLine();

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.3f, 0.8f, 0.3f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
		ImGui::PushFont(boldFont);
		if (ImGui::Button("Y", buttonSize))
			values.y = resetValue;
		ImGui::PopFont();
		ImGui::PopStyleColor(3);

		ImGui::SameLine();
		ImGui::DragFloat("##Y", &values.y, 0.1f, 0.0f, 0.0f, "%.2f");
		ImGui::PopItemWidth();
		ImGui::SameLine();

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.2f, 0.35f, 0.9f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });
		ImGui::PushFont(boldFont);
		if (ImGui::Button("Z", buttonSize))
			values.z = resetValue;
		ImGui::PopFont();
		ImGui::PopStyleColor(3);

		ImGui::SameLine();
		ImGui::DragFloat("##Z", &values.z, 0.1f, 0.0f, 0.0f, "%.2f");
		ImGui::PopItemWidth();

		ImGui::PopStyleVar();

		ImGui::PopID();

		ImGui::NextColumn();
	}

	template<typename T, typename UIFunction>
	static void DrawComponent(const std::string& name, Entity entity, UIFunction uiFunction)
	{
		const ImGuiTreeNodeFlags treeNodeFlags = ImGuiTreeNodeFlags_DefaultOpen
			| ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_SpanAvailWidth
			| ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_FramePadding;
		if (entity.HasComponent<T>())
		{
			auto& component = entity.GetComponent<T>();
			ImVec2 contentRegionAvailable = ImGui::GetContentRegionAvail();

			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{ 4, 4 });
			float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
			ImGui::Separator();
			// TODO: allow multi same component
			bool open = ImGui::TreeNodeEx(name.c_str(), treeNodeFlags);
			ImGui::PopStyleVar();
			ImGui::SameLine(contentRegionAvailable.x - lineHeight * 0.5f);
			if (ImGui::Button("+", ImVec2{ lineHeight, lineHeight }))
				ImGui::OpenPopup("ComponentSettings");

			bool removeComponent = false;
			if (ImGui::BeginPopup("ComponentSettings"))
			{
				if (ImGui::MenuItem("Remove component"))
					removeComponent = true;

				ImGui::EndPopup();
			}

			if (open)
			{
				uiFunction(component);
				ImGui::TreePop();
			}

			if (removeComponent)
				entity.RemoveComponent<T>();
		}
	}

	template<typename T>
	static void DisplayAddComponentEntry(const std::string& entryName, Entity entity)
	{
		if (!entity.HasComponent<T>())
		{
			if (ImGui::MenuItem(entryName.c_str()))
			{
				entity.AddComponent<T>();
				ImGui::CloseCurrentPopup();
			}
		}
	}

	void SceneHierarchyPanel::DrawComponents(Entity entity)
	{
		ImGui::Begin("Properties");
		if (!entity)
		{
			ImGui::End();
			return;
		}

		if (entity.HasComponent<BaseComponent>())
		{
			auto& name = entity.GetComponent<BaseComponent>().name;

			static char buf[256];
			strncpy_s(buf, sizeof(buf), name.c_str(), sizeof(buf));
			if (ImGui::InputText("##RenameInput", buf, IM_ARRAYSIZE(buf), ImGuiInputTextFlags_EnterReturnsTrue))
				if (strlen(buf) > 0 && buf != name) name = buf;
		}

		ImGui::SameLine();
		ImGui::PushItemWidth(-1);

		if (ImGui::Button("Add Component"))
			ImGui::OpenPopup("AddComponent");

		if (ImGui::BeginPopup("AddComponent"))
		{
			DisplayAddComponentEntry<CameraComponent>("Camera", entity);
			DisplayAddComponentEntry<ScriptComponent>("Script", entity);
			DisplayAddComponentEntry<LightComponent>("Light", entity);
			DisplayAddComponentEntry<SpriteRendererComponent>("Sprite Renderer", entity);
			DisplayAddComponentEntry<MeshRendererComponent>("Mesh Renderer", entity);

			ImGui::EndPopup();
		}

		ImGui::PopItemWidth();

		// TODO: support multi same components
		DrawComponent<TransformComponent>("Transform", entity, [](auto& component) {
			IMGUIRenderer::BeginPropertyGrid();

			DrawVec3Control("Translation", component.translation);
			DrawVec3Control("Rotation", component.rotation);
			DrawVec3Control("Scale", component.scale, 1.0f);

			IMGUIRenderer::EndPropertyGrid();
			});

		DrawComponent<LightComponent>("light", entity, [](auto& component) {
			IMGUIRenderer::BeginPropertyGrid();

			IMGUIRenderer::PropertyColor("Color", component.Color);
			IMGUIRenderer::Property("Intensity", component.Intensity, 0.01f, 0.f, 100.f);
			constexpr std::array<const char*, 3> lightTypeNames = { "Directional", "Point", "Spot" };
			IMGUIRenderer::PropertyDropdown("Type", lightTypeNames.data(), static_cast<int32_t>(lightTypeNames.size()), (int32_t*)&component.Type);

			if (component.Type == LightComponent::LightType::Spot)
			{
				IMGUIRenderer::Property("Cutoff", component.Cutoff, 0.1f, 1.f, 88.f);
				if (component.Cutoff > component.OuterCutoff)
					component.OuterCutoff = component.Cutoff + 0.5f;
				IMGUIRenderer::Property("OuterCutoff", component.OuterCutoff, 0.1f, component.Cutoff + 0.5f, 89.f);
			}

			IMGUIRenderer::EndPropertyGrid();
			});

		DrawComponent<SpriteRendererComponent>("Sprite Renderer", entity, [](auto& component) {
			ImGui::ColorEdit4("Color", glm::value_ptr(component.color));
			
			std::string label = "None";
			if (component.texture != 0)
			{
				if (LUMEN_ASSET_MANAGER->GetAssetType(component.texture) == AssetType::Texture)
					label = LUMEN_ASSET_MANAGER->GetAssetPath(component.texture).filename().string();
				else
					label = "Invalid";
			}

			ImVec2 buttonLabelSize = ImGui::CalcTextSize(label.c_str());
			buttonLabelSize.x += 20.0f;
			float buttonLabelWidth = glm::max<float>(100.0f, buttonLabelSize.x);

			ImGui::Button(label.c_str(), ImVec2(buttonLabelWidth, 0.0f));
			if (ImGui::BeginDragDropTarget())
			{
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
				{
					ASSERT(payload->DataSize == sizeof(AssetHandle));
					AssetHandle handle = *(AssetHandle*)payload->Data;
					if (LUMEN_ASSET_MANAGER->GetAssetType(handle) == AssetType::Texture)
					{
						component.texture = handle;
					}
				}
				ImGui::EndDragDropTarget();
			}
			ImGui::SameLine();
			ImGui::Text("Texture");

			ImGui::DragFloat("Tiling Factor", &component.tilingFactor, 0.1f, 0.0f, 100.0f);
			});

		DrawComponent<MeshRendererComponent>("Mesh Renderer", entity, [](auto& component) {
			IMGUIRenderer::BeginPropertyGrid();
		
			IMGUIRenderer::PropertyAssetReference("Mesh", component.Mesh, AssetType::Mesh);

			IMGUIRenderer::EndPropertyGrid();
			
			if (IMGUIRenderer::BeginTreeNode("Materials", false))
			{
				IMGUIRenderer::BeginPropertyGrid();
				for (size_t i = 0; i < component.Materials.size(); ++i)
				{
					std::string label = std::string("Material ") + std::to_string(i);
					IMGUIRenderer::PropertyAssetReference(label.c_str(), component.Materials[i], AssetType::Material);
				}
				IMGUIRenderer::EndPropertyGrid();

				if (ImGui::Button("Remove"))
				{
					component.Materials.pop_back();
				}
				ImGui::SameLine();
				if (ImGui::Button("Add Material"))
				{
					component.Materials.push_back(0);
				}

				IMGUIRenderer::EndTreeNode();
			}

			});

		/*DrawComponent<CameraComponent>("Camera", entity, [](auto& component) {
			auto& camera = component.Camera;

			ImGui::Checkbox("Primary", &component.Primary);

			const char* projectionTypeStrings[] = { "Perspective", "Orthographic" };
			const char* currentProjectionTypeString = projectionTypeStrings[(int)camera.GetProjectionType()];
			if (ImGui::BeginCombo("Projection", currentProjectionTypeString))
			{
				for (int i = 0; i < 2; i++)
				{
					bool isSelected = currentProjectionTypeString == projectionTypeStrings[i];
					if (ImGui::Selectable(projectionTypeStrings[i], isSelected))
					{
						currentProjectionTypeString = projectionTypeStrings[i];
						camera.SetProjectionType((SceneCamera::ProjectionType)i);
					}

					if (isSelected)
						ImGui::SetItemDefaultFocus();
				}

				ImGui::EndCombo();
			}

			if (camera.GetProjectionType() == SceneCamera::ProjectionType::Perspective)
			{
				float perspectiveVerticalFov = glm::degrees(camera.GetPerspectiveVerticalFOV());
				if (ImGui::DragFloat("Vertical FOV", &perspectiveVerticalFov))
					camera.SetPerspectiveVerticalFOV(glm::radians(perspectiveVerticalFov));

				float perspectiveNear = camera.GetPerspectiveNearClip();
				if (ImGui::DragFloat("Near", &perspectiveNear))
					camera.SetPerspectiveNearClip(perspectiveNear);

				float perspectiveFar = camera.GetPerspectiveFarClip();
				if (ImGui::DragFloat("Far", &perspectiveFar))
					camera.SetPerspectiveFarClip(perspectiveFar);
			}

			if (camera.GetProjectionType() == SceneCamera::ProjectionType::Orthographic)
			{
				float orthoSize = camera.GetOrthographicSize();
				if (ImGui::DragFloat("Size", &orthoSize))
					camera.SetOrthographicSize(orthoSize);

				float orthoNear = camera.GetOrthographicNearClip();
				if (ImGui::DragFloat("Near", &orthoNear))
					camera.SetOrthographicNearClip(orthoNear);

				float orthoFar = camera.GetOrthographicFarClip();
				if (ImGui::DragFloat("Far", &orthoFar))
					camera.SetOrthographicFarClip(orthoFar);

				ImGui::Checkbox("Fixed Aspect Ratio", &component.FixedAspectRatio);
			}
			});

		DrawComponent<ScriptComponent>("Script", entity, [entity, scene = m_Context](auto& component)
			{
				bool scriptClassExists = ScriptEngine::EntityClassExists(component.ClassName);

				static char buffer[64];
				strcpy_s(buffer, sizeof(buffer), component.ClassName.c_str());

				UI::ScopedStyleColor textColor(ImGuiCol_Text, ImVec4(0.9f, 0.2f, 0.3f, 1.0f), !scriptClassExists);

				if (ImGui::InputText("Class", buffer, sizeof(buffer)))
				{
					component.ClassName = buffer;
					return;
				}

				// Fields
				bool sceneRunning = scene->IsRunning();
				if (sceneRunning)
				{
					Ref<ScriptInstance> scriptInstance = ScriptEngine::GetEntityScriptInstance(entity.GetUUID());
					if (scriptInstance)
					{
						const auto& fields = scriptInstance->GetScriptClass()->GetFields();
						for (const auto& [name, field] : fields)
						{
							if (field.Type == ScriptFieldType::Float)
							{
								float data = scriptInstance->GetFieldValue<float>(name);
								if (ImGui::DragFloat(name.c_str(), &data))
								{
									scriptInstance->SetFieldValue(name, data);
								}
							}
						}
					}
				}
				else
				{
					if (scriptClassExists)
					{
						Ref<ScriptClass> entityClass = ScriptEngine::GetEntityClass(component.ClassName);
						const auto& fields = entityClass->GetFields();

						auto& entityFields = ScriptEngine::GetScriptFieldMap(entity);
						for (const auto& [name, field] : fields)
						{
							// Field has been set in editor
							if (entityFields.find(name) != entityFields.end())
							{
								ScriptFieldInstance& scriptField = entityFields.at(name);

								// Display control to set it maybe
								if (field.Type == ScriptFieldType::Float)
								{
									float data = scriptField.GetValue<float>();
									if (ImGui::DragFloat(name.c_str(), &data))
										scriptField.SetValue(data);
								}
							}
							else
							{
								// Display control to set it maybe
								if (field.Type == ScriptFieldType::Float)
								{
									float data = 0.0f;
									if (ImGui::DragFloat(name.c_str(), &data))
									{
										ScriptFieldInstance& fieldInstance = entityFields[name];
										fieldInstance.Field = field;
										fieldInstance.SetValue(data);
									}
								}
							}
						}
					}
				}
			});*/

		ImGui::End();
	}

	void SceneHierarchyPanel::DrawLighting(Lighting& lighting)
	{
		ImGui::Begin("Lighting");

		bool update = false;

		IMGUIRenderer::BeginPropertyGrid();

		if (IMGUIRenderer::PropertyAssetReference("Skybox Material", lighting.Skybox, AssetType::Material))
		{
			auto scene = LUMEN_ASSET_MANAGER->GetAsset<Scene>(m_Scene);
			scene->UpdateLighting();
		}
		
		IMGUIRenderer::EndPropertyGrid();
		
		ImGui::End();
	}
}