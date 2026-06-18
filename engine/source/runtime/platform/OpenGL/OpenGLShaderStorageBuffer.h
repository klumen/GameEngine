#pragma once

#include "runtime/function/render/ShaderStorageBuffer.h"

namespace Lumen
{
	class OpenGLShaderStorageBuffer : public ShaderStorageBuffer
	{
	public:
		OpenGLShaderStorageBuffer();
		virtual ~OpenGLShaderStorageBuffer();

	private:
		uint32_t m_RendererID;

	};
}