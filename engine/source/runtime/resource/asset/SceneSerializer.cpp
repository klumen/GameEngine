#include "runtime/resource/asset/SceneSerializer.h"

#include "runtime/resource/data/Scene.h"
#include "runtime/resource/data/Entity.h"
#include "runtime/resource/asset/Serializer.h"

#include <fstream>

namespace Lumen
{
	SceneSerializer::SceneSerializer(const Shared<Scene>& scene)
		: m_Scene(scene) { }

	bool SceneSerializer::Serialize(const std::filesystem::path& scenePath)
	{
		YAML::Emitter out;
		out << YAML::BeginMap;
		out << YAML::Key << "Scene" << YAML::Value;
		{
			out << YAML::BeginMap;
			out << YAML::Key << "Entities" << YAML::Value << YAML::BeginSeq;
			m_Scene->GetRegistry().view<entt::entity>().each([&](auto entityID) {
				Entity entity = { entityID, m_Scene };
				if (!entity)
					return;

				SerializeEntity(out, entity);
				});
			out << YAML::EndSeq;
			out << YAML::EndMap;
		}
		out << YAML::EndMap;

		std::ofstream fout(scenePath);
		if (fout.is_open())
		{
			fout << out.c_str();
			fout.close();
			return true;
		}
		else
			return false;
	}

	bool SceneSerializer::Deserialize(const std::filesystem::path& scenePath)
	{
		YAML::Node data = YAML::LoadFile(scenePath.string());
		auto sceneNode = data["Scene"];
		if (!sceneNode)
			return false;

		if (auto entities = sceneNode["Entities"])
		{
			for (auto entity : entities)
			{
				uint64_t uuid = 0;
				std::string name;
				auto baseComponent = entity["Base"];
				if (baseComponent)
				{
					// Entities always have base
					uuid = baseComponent["UUID"].as<uint64_t>();
					name = baseComponent["Name"].as<std::string>();
				}

				//LUMEN_RUNTIME_TRACE("Deserialized entity with ID = {0}, name = {1}", uuid, name);

				Entity deserializedEntity = m_Scene->CreateEntityWithUUID(uuid, name);

				auto transformComponent = entity["Transform"];
				if (transformComponent)
				{
					// Entities always have transform
					auto& transform = deserializedEntity.GetComponent<TransformComponent>();
					transform.translation = transformComponent["Translation"].as<glm::vec3>();
					transform.rotation = transformComponent["Rotation"].as<glm::vec3>();
					transform.scale = transformComponent["Scale"].as<glm::vec3>();
				}

				auto cameraComponent = entity["CameraComponent"];
				if (cameraComponent)
				{
					/*auto& cc = deserializedEntity.AddComponent<CameraComponent>();

					auto& cameraProps = cameraComponent["Camera"];
					cc.Camera.SetProjectionType((SceneCamera::ProjectionType)cameraProps["ProjectionType"].as<int>());

					cc.Camera.SetPerspectiveVerticalFOV(cameraProps["PerspectiveFOV"].as<float>());
					cc.Camera.SetPerspectiveNearClip(cameraProps["PerspectiveNear"].as<float>());
					cc.Camera.SetPerspectiveFarClip(cameraProps["PerspectiveFar"].as<float>());

					cc.Camera.SetOrthographicSize(cameraProps["OrthographicSize"].as<float>());
					cc.Camera.SetOrthographicNearClip(cameraProps["OrthographicNear"].as<float>());
					cc.Camera.SetOrthographicFarClip(cameraProps["OrthographicFar"].as<float>());

					cc.Primary = cameraComponent["Primary"].as<bool>();
					cc.FixedAspectRatio = cameraComponent["FixedAspectRatio"].as<bool>();*/
				}

				auto scriptComponent = entity["ScriptComponent"];
				if (scriptComponent)
				{
					//auto& sc = deserializedEntity.AddComponent<ScriptComponent>();
					//sc.ClassName = scriptComponent["ClassName"].as<std::string>();

					//auto scriptFields = scriptComponent["ScriptFields"];
					//if (scriptFields)
					//{
					//	Ref<ScriptClass> entityClass = ScriptEngine::GetEntityClass(sc.ClassName);
					//	if (entityClass)
					//	{
					//		const auto& fields = entityClass->GetFields();
					//		auto& entityFields = ScriptEngine::GetScriptFieldMap(deserializedEntity);

					//		for (auto scriptField : scriptFields)
					//		{
					//			std::string name = scriptField["Name"].as<std::string>();
					//			std::string typeString = scriptField["Type"].as<std::string>();
					//			ScriptFieldType type = Utils::ScriptFieldTypeFromString(typeString);

					//			ScriptFieldInstance& fieldInstance = entityFields[name];

					//			// TODO(Yan): turn this assert into Hazelnut log warning
					//			HZ_CORE_ASSERT(fields.find(name) != fields.end());

					//			if (fields.find(name) == fields.end())
					//				continue;

					//			fieldInstance.Field = fields.at(name);

					//			switch (type)
					//			{
					//				READ_SCRIPT_FIELD(Float, float);
					//				READ_SCRIPT_FIELD(Double, double);
					//				READ_SCRIPT_FIELD(Bool, bool);
					//				READ_SCRIPT_FIELD(Char, char);
					//				READ_SCRIPT_FIELD(Byte, int8_t);
					//				READ_SCRIPT_FIELD(Short, int16_t);
					//				READ_SCRIPT_FIELD(Int, int32_t);
					//				READ_SCRIPT_FIELD(Long, int64_t);
					//				READ_SCRIPT_FIELD(UByte, uint8_t);
					//				READ_SCRIPT_FIELD(UShort, uint16_t);
					//				READ_SCRIPT_FIELD(UInt, uint32_t);
					//				READ_SCRIPT_FIELD(ULong, uint64_t);
					//				READ_SCRIPT_FIELD(Vector2, glm::vec2);
					//				READ_SCRIPT_FIELD(Vector3, glm::vec3);
					//				READ_SCRIPT_FIELD(Vector4, glm::vec4);
					//				READ_SCRIPT_FIELD(Entity, UUID);
					//			}
					//		}
					//	}
					//}
				}

				auto spriteRendererComponent = entity["SpriteRendererComponent"];
				if (spriteRendererComponent)
				{
					//auto& src = deserializedEntity.AddComponent<SpriteRendererComponent>();
					//src.Color = spriteRendererComponent["Color"].as<glm::vec4>();
					//if (spriteRendererComponent["TexturePath"])
					//{
					//	// NOTE(Yan): legacy, could try and find something in the asset registry that matches?
					//	// std::string texturePath = spriteRendererComponent["TexturePath"].as<std::string>();
					//	// auto path = Project::GetAssetFileSystemPath(texturePath);
					//	// src.Texture = Texture::Create(path.string());
					//}

					//if (spriteRendererComponent["TextureHandle"])
					//	src.Texture = spriteRendererComponent["TextureHandle"].as<AssetHandle>();

					//if (spriteRendererComponent["TilingFactor"])
					//	src.TilingFactor = spriteRendererComponent["TilingFactor"].as<float>();
				}
			}
		}

		return true;
	}

	bool SceneSerializer::SerializeEntity(YAML::Emitter& out, Entity entity)
	{
		out << YAML::BeginMap; // Entity

		if (entity.HasComponent<BaseComponent>())
		{
			out << YAML::Key << "Base";
			out << YAML::BeginMap; // Base

			auto& base = entity.GetComponent<BaseComponent>();
			out << YAML::Key << "UUID" << YAML::Value << (uint64_t)base.id;
			out << YAML::Key << "Name" << YAML::Value << base.name;

			out << YAML::EndMap; // Base
		}

		if (entity.HasComponent<TransformComponent>())
		{
			out << YAML::Key << "Transform";
			out << YAML::BeginMap; // Transform

			auto& trans = entity.GetComponent<TransformComponent>();
			out << YAML::Key << "Translation" << YAML::Value << trans.translation;
			out << YAML::Key << "Rotation" << YAML::Value << trans.rotation;
			out << YAML::Key << "Scale" << YAML::Value << trans.scale;

			out << YAML::EndMap; // Transform
		}

		//if (entity.HasComponent<CameraComponent>())
		//{
		//	out << YAML::Key << "CameraComponent";
		//	out << YAML::BeginMap; // CameraComponent

		//	auto& cameraComponent = entity.GetComponent<CameraComponent>();
		//	auto& camera = cameraComponent.Camera;

		//	out << YAML::Key << "Camera" << YAML::Value;
		//	out << YAML::BeginMap; // Camera
		//	out << YAML::Key << "ProjectionType" << YAML::Value << (int)camera.GetProjectionType();
		//	out << YAML::Key << "PerspectiveFOV" << YAML::Value << camera.GetPerspectiveVerticalFOV();
		//	out << YAML::Key << "PerspectiveNear" << YAML::Value << camera.GetPerspectiveNearClip();
		//	out << YAML::Key << "PerspectiveFar" << YAML::Value << camera.GetPerspectiveFarClip();
		//	out << YAML::Key << "OrthographicSize" << YAML::Value << camera.GetOrthographicSize();
		//	out << YAML::Key << "OrthographicNear" << YAML::Value << camera.GetOrthographicNearClip();
		//	out << YAML::Key << "OrthographicFar" << YAML::Value << camera.GetOrthographicFarClip();
		//	out << YAML::EndMap; // Camera

		//	out << YAML::Key << "Primary" << YAML::Value << cameraComponent.Primary;
		//	out << YAML::Key << "FixedAspectRatio" << YAML::Value << cameraComponent.FixedAspectRatio;

		//	out << YAML::EndMap; // CameraComponent
		//}

		//	out << YAML::EndMap; // ScriptComponent
		//}

		//if (entity.HasComponent<SpriteRendererComponent>())
		//{
		//	out << YAML::Key << "SpriteRendererComponent";
		//	out << YAML::BeginMap; // SpriteRendererComponent

		//	auto& spriteRendererComponent = entity.GetComponent<SpriteRendererComponent>();
		//	out << YAML::Key << "Color" << YAML::Value << spriteRendererComponent.Color;
		//	out << YAML::Key << "TextureHandle" << YAML::Value << spriteRendererComponent.Texture;

		//	out << YAML::Key << "TilingFactor" << YAML::Value << spriteRendererComponent.TilingFactor;

		//	out << YAML::EndMap; // SpriteRendererComponent
		//}

		out << YAML::EndMap; // Entity
		return true;
	}
}