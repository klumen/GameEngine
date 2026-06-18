#pragma once

#include "runtime/core/Memory.h"

namespace Lumen
{
	class ShaderStorageBuffer
	{
	public:
		virtual ~ShaderStorageBuffer() = default;

		static Shared<ShaderStorageBuffer> Create();

	private:

	};
}