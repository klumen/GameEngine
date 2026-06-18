#pragma once

#include "runtime/resource/asset/Asset.h"
#include "runtime/core/Memory.h"

#include <entt.hpp>
#include <glm/glm.hpp>

#include <unordered_map>

namespace Lumen
{
	struct Lighting
	{
		AssetHandle Skybox = 0;
	};

	class Entity;
	using EntityMap = std::unordered_map<UUID, entt::entity>;

	class Scene : public Asset, public std::enable_shared_from_this<Scene>
	{
		friend class Entity;

	public:
		Scene() = default;
		virtual ~Scene() = default;

		static AssetType GetStaticType() { return AssetType::Scene; }
		virtual AssetType GetType() const override { return GetStaticType(); };

		void Update();
		void UpdateLighting() const;

		Entity CreateEntity(const std::string& name);
		Entity CreateEntityWithUUID(UUID id, const std::string& name);
		void DestroyEntity(Entity entity);

		entt::registry& GetRegistry() { return m_Registry; }
		const EntityMap& GetEntityMap() const { return m_EntityMap; }
		Lighting& GetLighting() { return m_Lighting; }

		glm::mat4 UpdateTransform(Entity entity);

		void SetSkybox(AssetHandle skybox);

	private:
		entt::registry m_Registry;
		EntityMap m_EntityMap;

		Lighting m_Lighting;

	};
}