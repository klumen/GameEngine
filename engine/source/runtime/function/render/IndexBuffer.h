#pragma once

#include "runtime/core/Memory.h"

namespace Lumen
{
	class IndexBuffer
	{
	public:
		virtual ~IndexBuffer() = default;
		
		// TODO: uint32_t* uint16_t* uint8_t* data
		static Shared<IndexBuffer> Create(const uint32_t* data, uint32_t count);

		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;
		virtual uint32_t GetCount() const = 0;

	};
}