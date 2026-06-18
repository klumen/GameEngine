#include "runtime/resource/data/Scene.h"

#include "runtime/function/global/GlobalContext.h"
#include "runtime/function/render/RenderSystem.h"
#include "runtime/resource/data/Entity.h"

namespace Lumen
{
	void Scene::Update()
	{
		{
			auto view = m_Registry.view<ScriptComponent>();
			for (auto entity : view)
			{
				auto& script = view.get<ScriptComponent>(entity);
			}
		}

		{
			auto view = m_Registry.view<TransformComponent>();
			for (auto entity : view)
			{
				auto& transform = view.get<TransformComponent>(entity);
				transform.isDirty = true;
				// TODO: use handle
				UpdateTransform(Entity(entity, weak_from_this()));
			}
		}

		{
			auto view = m_Registry.view<TransformComponent, LightComponent>();
			for (auto entity : view)
			{
				auto [transform, light] = view.get<TransformComponent, LightComponent>(entity);
				light.Position = transform.translation;
				light.Direction = glm::rotate(glm::quat(glm::radians(transform.rotation)), glm::vec3(0.f, 0.f, -1.f));
				LUMEN_RENDER_SYSTEM->SubmitLight(light);
			}
		}

		{
			auto view = m_Registry.view<TransformComponent, MeshRendererComponent>();
			for (auto entity : view)
			{
				auto [transform, meshRenderer] = view.get<TransformComponent, MeshRendererComponent>(entity);
				if (!meshRenderer.Mesh)
					continue;
				LUMEN_RENDER_SYSTEM->SubmitMesh(transform.GetTransform(), meshRenderer);
			}
		}

		{
			auto view = m_Registry.view<TransformComponent, SpriteRendererComponent>();
			for (auto entity : view)
			{
				auto [transform, spriteRenderer] = view.get<TransformComponent, SpriteRendererComponent>(entity);
				LUMEN_RENDER_SYSTEM->SubmitSprite(transform.GetTransform(), spriteRenderer);
			}
		}
	}

	void Scene::UpdateLighting() const
	{
		LUMEN_RENDER_SYSTEM->SetSkybox(m_Lighting.Skybox);
	}

	Entity Scene::CreateEntity(const std::string& name)
	{
		return CreateEntityWithUUID(UUID(), name);
	}

	Entity Scene::CreateEntityWithUUID(UUID uuid, const std::string& name)
	{
		Entity entity(m_Registry.create(), weak_from_this());
		auto& base = entity.AddComponent<BaseComponent>();
		base.id = uuid;
		base.name = name.empty() ? "Entity" : name;
		entity.AddComponent<TransformComponent>();

		m_EntityMap[uuid] = entity;
		return entity;
	}

	void Scene::DestroyEntity(Entity entity)
	{
		m_EntityMap.erase(entity.GetUUID());
		m_Registry.destroy(entity);
	}

	glm::mat4 Scene::UpdateTransform(Entity entity)
	{
		auto& trans = entity.GetComponent<TransformComponent>();
		if (trans.isDirty)
		{
			const glm::mat4 localTransform =
				glm::translate(glm::mat4(1.0f), trans.translation)
				* glm::toMat4(glm::quat(glm::radians(trans.rotation)))
				* glm::scale(glm::mat4(1.0f), trans.scale);

			if (trans.parent)
			{
				trans.transform = UpdateTransform(Entity(m_EntityMap[trans.parent], weak_from_this())) * localTransform;
			}
			else
				trans.transform = localTransform;

			trans.isDirty = false;
		}

		return trans.transform;
	}

	void Scene::SetSkybox(AssetHandle skybox)
	{
		m_Lighting.Skybox = skybox;
	}
}