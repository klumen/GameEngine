#pragma once

#include "runtime/resource/data/Scene.h"
#include "runtime/resource/data/Component.h"
#include "runtime/core/Memory.h"
#include "runtime/core/Macro.h"

#include <entt.hpp>

namespace Lumen
{
	class Entity
	{
	public:
		Entity() = default;
		Entity(entt::entity handle, const Weak<Scene>& scene)
			: m_Entity(handle), m_Scene(scene) {}

		// TODO: allow to add the same component
		template<typename T, typename... Args>
		T& AddComponent(Args&&... args)
		{
			ASSERT(!HasComponent<T>());
			T& component = m_Scene.lock()->m_Registry.emplace<T>(m_Entity, std::forward<Args>(args)...);
			return component;
		}

		template<typename T, typename... Args>
		T& AddOrReplaceComponent(Args&&... args)
		{
			T& component = m_Scene.lock()->m_Registry.emplace_or_replace<T>(m_Entity, std::forward<Args>(args)...);
			return component;
		}

		template<typename T>
		T& GetComponent()
		{
			ASSERT(HasComponent<T>());
			return m_Scene.lock()->m_Registry.get<T>(m_Entity);
		}

		template<typename T>
		bool HasComponent()
		{
			return m_Scene.lock()->m_Registry.all_of<T>(m_Entity);
		}

		template<typename T>
		void RemoveComponent()
		{
			ASSERT(HasComponent<T>());
			m_Scene.lock()->m_Registry.remove<T>(m_Entity);
		}

		bool operator==(const Entity& other) const
		{
			return m_Entity == other.m_Entity && m_Scene.lock() == other.m_Scene.lock();
		}

		bool operator!=(const Entity& other) const
		{
			return m_Entity != other.m_Entity || m_Scene.lock() != other.m_Scene.lock();
		}

		operator bool() const { return m_Entity != entt::null; }
		operator entt::entity() const { return m_Entity; }
		operator uint32_t() const { return (uint32_t)m_Entity; }

		UUID GetUUID() { return GetComponent<BaseComponent>().id; }
		const std::string& GetName() { return GetComponent<BaseComponent>().name; }

	private:
		entt::entity m_Entity{ entt::null };
		Weak<Scene> m_Scene;

	};
}