#pragma once

#include "runtime/resource/asset/Asset.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

#include <string>
#include <vector>

namespace Lumen
{
	struct BaseComponent
	{
		UUID id;
		std::string name;
	};

	struct TransformComponent
	{
		// local
		glm::vec3 translation = { 0.f, 0.f, 0.f };
		glm::vec3 rotation = { 0.f, 0.f, 0.f }; // euler(degree)
		glm::vec3 scale = { 1.f, 1.f, 1.f };

		// global
		glm::mat4 transform{ 1.f };
		bool isDirty = true;

		UUID parent = 0;
		std::vector<UUID> children;

		void MarkDirty() { isDirty = true; }

		glm::mat4 GetTransform() const { return transform; }
	};

	struct CameraComponent
	{
		enum class ProjectionType { Perspective = 0, Orthographic = 1 };

		// if clear previous color or depth buffer 
		enum class ClearFlag
		{
			SOLID_COLOR, // clear color and depth buffer replace color with background color;
			SKYBOX, // clear color and depth buffer, replace color with skybox color
			DEPTH_ONLY, // only clear depth buffer
			NO_CLEAR // reserve color and depth
		};

		ProjectionType ProjectionType = ProjectionType::Orthographic;
		ClearFlag ClearFlag = ClearFlag::SOLID_COLOR;

		float Size = 5.f;
		float FOV = 60.f;
		float Aspect = 1.f;
		float Near = 0.3f, m_Far = 1000.f;

		glm::mat4 View{ 1.f };
		glm::mat4 Projection{ 1.f };
	};

	struct LightComponent
	{
		enum class LightType { Directional = 0, Point = 1, Spot = 2 };

		LightType Type = LightType::Directional;

		glm::vec3 Position{ 0.f, 0.f, 0.f };
		glm::vec3 Direction{ 0.f, 0.f, 1.f };

		glm::vec3 Color{ 1.f };
		float Intensity = 2.f;

		/*float Constant = 1.f;
		float Linear = 0.09f;
		float Quadratic = 0.032f;*/

		float Cutoff = 12.5f;
		float OuterCutoff = 15.f;
	};

	struct SpriteRendererComponent
	{
		AssetHandle texture = 0;
		glm::vec4 color{ 1.0f, 1.0f, 1.0f, 1.0f };
		float tilingFactor = 1.0f;
		AssetHandle material = 0;
	};

	struct MeshRendererComponent
	{
		AssetHandle Mesh = 0;
		std::vector<AssetHandle> Materials; // for submesh
	};

	struct ScriptComponent
	{
		std::string className;
	};
}