#pragma once

#include "runtime/function/render/VertexBuffer.h"
#include "runtime/function/render/IndexBuffer.h"
#include "runtime/resource/asset/Asset.h"

#include <glm/glm.hpp>

#include <vector>

namespace Lumen
{
	struct Vertex
	{
		glm::vec3 Position;
		glm::vec3 Normal;
		glm::vec3 Tangent;
		glm::vec3 Binormal;
		glm::vec2 Texcoord;
	};

	struct AnimatedVertex
	{
		glm::vec3 Position;
		glm::vec3 Normal;
		glm::vec3 Tangent;
		glm::vec3 Binormal;
		glm::vec2 Texcoord;

		uint32_t IDs[4] = { 0 };
		float Weights[4] = { 0.0f };
	};

	struct Submesh
	{
		uint32_t StartVertex = 0;
		uint32_t VertexCount = 0;

		uint32_t StartIndex = 0;
		uint32_t IndexCount = 0;
		
		//AABB BoundingBox;
	};

	class Mesh : public Asset
	{
	public:
		Mesh() = default;
		Mesh(const std::vector<Submesh>& submeshes, const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices);

		static AssetType GetStaticType() { return AssetType::Mesh; }
		virtual AssetType GetType() const override { return GetStaticType(); };

		Shared<VertexBuffer>& GetVBO() { return m_VBO; }
		Shared<IndexBuffer>& GetIBO() { return m_IBO; }
		const std::vector<Submesh>& GetSubmesh() const { return m_Submeshes; }

		void Update();

	private:
		Shared<VertexBuffer> m_VBO;
		Shared<IndexBuffer> m_IBO;
		
		std::vector<Submesh> m_Submeshes;

		std::vector<Vertex> m_Vertices;
		std::vector<uint32_t> m_Indices;

	};
}