#include "runtime/platform/OpenGL/OpenGLFrameBuffer.h"

#include "runtime/function/global/GlobalContext.h"
#include "runtime/core/Log.h"

#include <glad/glad.h>

namespace Lumen
{
	static bool IsDepthFormat(ImageFormat format)
	{
		switch (format)
		{
		case ImageFormat::DEPTH24_STENCIL8: return true;
		case ImageFormat::DEPTH32F: return true;
		case ImageFormat::DEPTH32F_STENCIL8: return true;
		}
		return false;
	}

	static void AttachTexture2D(uint32_t id, int samples, GLenum internalFormat, GLenum attachmentType, uint32_t width, uint32_t height)
	{
		bool multisampled = samples > 1;
		if (multisampled)
		{
			glTextureStorage2DMultisample(id, samples, internalFormat, width, height, GL_FALSE);
		}
		else
		{
			glTextureStorage2D(id, 1, internalFormat, width, height);

			if (attachmentType == GL_DEPTH_ATTACHMENT)
			{
				glTextureParameteri(id, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
				glTextureParameteri(id, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
				glTextureParameteri(id, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
				glTextureParameteri(id, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
				float borderColor[] = { 1.f, 1.f, 1.f, 1.f };
				glTextureParameterfv(id, GL_TEXTURE_BORDER_COLOR, borderColor);
			}
			else
			{
				glTextureParameteri(id, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
				glTextureParameteri(id, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
				glTextureParameteri(id, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
				glTextureParameteri(id, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			}
		}

		glFramebufferTexture2D(GL_FRAMEBUFFER, 
			attachmentType, 
			multisampled ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D, 
			id, 
			0);
	}

	static void AttachCubeMap(uint32_t id, GLenum internalFormat, GLenum attachmentType, uint32_t width, uint32_t height)
	{
		glTextureStorage2D(id, 1, internalFormat, width, height);

		glTextureParameteri(id, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTextureParameteri(id, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTextureParameteri(id, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTextureParameteri(id, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTextureParameteri(id, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

		glFramebufferTexture(GL_FRAMEBUFFER,
			attachmentType,
			id,
			0);
	}

	static const uint32_t s_MaxFramebufferSize = 8192;

	OpenGLFrameBuffer::OpenGLFrameBuffer(const FrameBufferInfo& info)
		: m_Info(info)
	{
		for (const auto& attaInfo : m_Info.Attachments)
		{
			if (IsDepthFormat(attaInfo.format))
				m_DepthAttaInfo = attaInfo;
			else
				m_ColorAttaInfo.emplace_back(attaInfo);
		}

		Flush();
	}

	OpenGLFrameBuffer::~OpenGLFrameBuffer()
	{
		glDeleteFramebuffers(1, &m_RendererID);
		glDeleteTextures((int)m_ColorAttachments.size(), m_ColorAttachments.data());
		glDeleteTextures(1, &m_DepthAttachment);
	}

	void OpenGLFrameBuffer::Flush()
	{
		if (m_RendererID)
		{
			glDeleteFramebuffers(1, &m_RendererID);
			glDeleteTextures((int)m_ColorAttachments.size(), m_ColorAttachments.data());
			glDeleteTextures(1, &m_DepthAttachment);

			m_ColorAttachments.clear();
			m_DepthAttachment = 0;
		}

		glCreateFramebuffers(1, &m_RendererID);
		glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID);

		bool multisampled = m_Info.Samples > 1;

		// Attachments
		if (m_ColorAttaInfo.size())
		{
			// TODO: CubeMap
			m_ColorAttachments.resize(m_ColorAttaInfo.size());
			glCreateTextures(multisampled ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D,
				(int)m_ColorAttachments.size(), m_ColorAttachments.data());

			for (size_t i = 0; i < m_ColorAttachments.size(); i++)
			{
				switch (m_ColorAttaInfo[i].format)
				{
				case ImageFormat::RGBA8:
					AttachTexture2D(m_ColorAttachments[i], m_Info.Samples, GL_RGBA8, GL_COLOR_ATTACHMENT0 + (int)i, m_Info.Width, m_Info.Height);
					break;
				case ImageFormat::RGBA16F:
					AttachTexture2D(m_ColorAttachments[i], m_Info.Samples, GL_RGBA16F, GL_COLOR_ATTACHMENT0 + (int)i, m_Info.Width, m_Info.Height);
					break;
				case ImageFormat::RGBA32F:
					AttachTexture2D(m_ColorAttachments[i], m_Info.Samples, GL_RGBA32F, GL_COLOR_ATTACHMENT0 + (int)i, m_Info.Width, m_Info.Height);
					break;
				case ImageFormat::RGB10_A2:
					AttachTexture2D(m_ColorAttachments[i], m_Info.Samples, GL_RGB10_A2, GL_COLOR_ATTACHMENT0 + (int)i, m_Info.Width, m_Info.Height);
					break;
				case ImageFormat::R32UI:
					// TODO:
					break;
				}
			}
		}

		if (m_DepthAttaInfo.format != ImageFormat::None)
		{
			if (m_DepthAttaInfo.shape == ImageShape::Texture2D)
			{
				glCreateTextures(multisampled ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D,
					1, &m_DepthAttachment);
				switch (m_DepthAttaInfo.format)
				{
				case ImageFormat::DEPTH24_STENCIL8:
					AttachTexture2D(m_DepthAttachment, m_Info.Samples, GL_DEPTH24_STENCIL8, GL_DEPTH_STENCIL_ATTACHMENT, m_Info.Width, m_Info.Height);
					break;
				case ImageFormat::DEPTH32F:
					AttachTexture2D(m_DepthAttachment, m_Info.Samples, GL_DEPTH_COMPONENT32F, GL_DEPTH_ATTACHMENT, m_Info.Width, m_Info.Height);
					break;
				case ImageFormat::DEPTH32F_STENCIL8:
					AttachTexture2D(m_DepthAttachment, m_Info.Samples, GL_DEPTH32F_STENCIL8, GL_DEPTH_STENCIL_ATTACHMENT, m_Info.Width, m_Info.Height);
					break;
				}
			}
			else if (m_DepthAttaInfo.shape == ImageShape::Cubemap)
			{
				glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &m_DepthAttachment);
				switch (m_DepthAttaInfo.format)
				{
				case ImageFormat::DEPTH24_STENCIL8:
					AttachCubeMap(m_DepthAttachment, GL_DEPTH24_STENCIL8, GL_DEPTH_STENCIL_ATTACHMENT, m_Info.Width, m_Info.Height);
					break;
				case ImageFormat::DEPTH32F:
					AttachCubeMap(m_DepthAttachment, GL_DEPTH_COMPONENT32F, GL_DEPTH_ATTACHMENT, m_Info.Width, m_Info.Height);
					break;
				case ImageFormat::DEPTH32F_STENCIL8:
					AttachCubeMap(m_DepthAttachment, GL_DEPTH32F_STENCIL8, GL_DEPTH_STENCIL_ATTACHMENT, m_Info.Width, m_Info.Height);
					break;
				}
			}
		}

		if (m_ColorAttachments.size() > 1)
		{
			ASSERT(m_ColorAttachments.size() <= 4);
			GLenum buffers[4] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3 };
			glDrawBuffers((int)m_ColorAttachments.size(), buffers);
		}
		else if (m_ColorAttachments.empty())
		{
			// Only depth-pass
			glDrawBuffer(GL_NONE);
			glReadBuffer(GL_NONE);
		}

		ASSERT(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void OpenGLFrameBuffer::Bind()
	{
		glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID);
		glViewport(0, 0, m_Info.Width, m_Info.Height);
	}

	void OpenGLFrameBuffer::Unbind()
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void OpenGLFrameBuffer::Resize(uint32_t width, uint32_t height)
	{
		if (width == 0 || height == 0 || width > s_MaxFramebufferSize || height > s_MaxFramebufferSize)
		{
			LUMEN_RUNTIME_ERROR("Attempted to rezize framebuffer to {0}, {1}", width, height);
			return;
		}
		m_Info.Width = width;
		m_Info.Height = height;

		Flush();
	}

	void OpenGLFrameBuffer::ClearColor(int drawbuffer, const int* value)
	{
		glClearNamedFramebufferiv(m_RendererID, GL_COLOR, drawbuffer, value);
	}

	void OpenGLFrameBuffer::ClearColor(int drawbuffer, const unsigned* value)
	{
		glClearNamedFramebufferuiv(m_RendererID, GL_COLOR, drawbuffer, value);
	}

	void OpenGLFrameBuffer::ClearColor(int drawbuffer, const float* value)
	{
		glClearNamedFramebufferfv(m_RendererID, GL_COLOR, drawbuffer, value);
	}

	void OpenGLFrameBuffer::ClearDepth(const float* value)
	{
		glClearNamedFramebufferfv(m_RendererID, GL_DEPTH, 0, value);
	}

	void OpenGLFrameBuffer::ClearStencil(const int* value)
	{
		glClearNamedFramebufferiv(m_RendererID, GL_STENCIL, 0, value);
	}

	void OpenGLFrameBuffer::ClearDepthStencil(float depth, int stencil)
	{
		glClearNamedFramebufferfi(m_RendererID, GL_DEPTH_STENCIL, 0, depth, stencil);
	}
}