#pragma once

#include "runtime/core/Memory.h"

namespace Lumen
{
	class UniformBuffer
	{
	public:
		virtual ~UniformBuffer() = default;

		static Shared<UniformBuffer> Create(uint32_t size, uint32_t binding);

		virtual void SetData(const void* data, uint32_t size, uint32_t offset) = 0;

	};
}