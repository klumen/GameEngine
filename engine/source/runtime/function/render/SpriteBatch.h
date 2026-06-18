#pragma once

#include "runtime/function/render/VertexBuffer.h"
#include "runtime/function/render/IndexBuffer.h"
#include "runtime/resource/data/Texture.h"

#include <glm/glm.hpp>

#include <array>

namespace Lumen
{
	struct SpriteVertex
	{
		glm::vec3 position;
		glm::vec4 color;
		glm::vec2 texCoord;
		float texIndex;
		float tilingFactor;
	};

	class SpriteBatch
	{
	public:
		static constexpr uint32_t MaxSprites = 10000;
		static constexpr uint32_t MaxVertices = MaxSprites * 4;
		static constexpr uint32_t MaxIndices = MaxSprites * 6;
		static constexpr uint32_t MaxTextureSlots = 32;

		Shared<VertexBuffer> VBO;
		Shared<IndexBuffer> IBO;

		std::vector<SpriteVertex> Vertices;
		std::vector<uint32_t> Indices; // const
		uint32_t IndexCount = 0; // ture count

		// TODO: maybe use AssetHandle
		std::array<Shared<Texture>, MaxTextureSlots> textureSlots;
		uint32_t textureSlotIndex = 1;

	private:

	};
}