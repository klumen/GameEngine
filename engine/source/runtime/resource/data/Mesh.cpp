#include "runtime/resource/data/Mesh.h"

namespace Lumen
{
	Mesh::Mesh(const std::vector<Submesh>& submeshes, const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices)
		: m_Submeshes(submeshes), m_Vertices(vertices), m_Indices(indices)
	{
		m_VBO = VertexBuffer::Create(m_Vertices.data(), static_cast<uint32_t>(m_Vertices.size() * sizeof(Vertex)));
		m_IBO = IndexBuffer::Create(m_Indices.data(), static_cast<uint32_t>(m_Indices.size()));
	}

	void Mesh::Update()
	{
		m_VBO = VertexBuffer::Create(m_Vertices.data(), static_cast<uint32_t>(m_Vertices.size() * sizeof(Vertex)));
		m_IBO = IndexBuffer::Create(m_Indices.data(), static_cast<uint32_t>(m_Indices.size()));
	}
}