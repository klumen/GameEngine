#include "runtime/function/render/Window.h"

#include "runtime/function/HID/KeyEvent.h"
#include "runtime/function/HID/MouseEvent.h"
#include "runtime/function/HID/ApplicationEvent.h"
#include "runtime/function/global/GlobalContext.h"
#include "runtime/resource/config/ConfigManager.h"
#include "runtime/core/Log.h"
#include "runtime/core/Macro.h"

#define GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace Lumen
{
	void Window::StartUp(const WindowInfo& info/* = WindowInfo{}*/)
	{
		LUMEN_RUNTIME_INFO("Creating window {0} ({1}, {2})", info.title, info.width, info.height);
		m_Data.width = info.width;
		m_Data.height = info.height;

		glfwSetErrorCallback([](int error, const char* description) {
			LUMEN_RUNTIME_ERROR("GLFW Error ({0}): {1}", error, description);
			});

		if (!glfwInit())
		{
			LUMEN_RUNTIME_CRITICAL("Failed to initialize GLFW window!");
			return;
		}

		switch (LUMEN_GRAPHICS_API)
		{
		case GraphicsAPI::OpenGL:
		{
			glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
			glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
			glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
		#ifdef LUMEN_DEBUG 
			glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
		#endif
			break;
		}
		case GraphicsAPI::Vulkan:
		{
			glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
			break;
		}
		ASSERT(false);
		}

		m_Window = glfwCreateWindow(info.width, info.height, info.title, nullptr, nullptr);
		if (!m_Window)
		{
			LUMEN_RUNTIME_CRITICAL("Failed to create GLFW window!");
			glfwTerminate();
			return;
		}

		// Set GLFW callbacks
		glfwSetWindowUserPointer(m_Window, &m_Data);
		
		glfwSetWindowSizeCallback(m_Window, [](GLFWwindow* window, int width, int height) {
			auto data = (WindowData*)glfwGetWindowUserPointer(window);
			data->width = width;
			data->height = height;

			WindowResizeEvent event(width, height);
			data->EventCallback(event);
			});
		
		glfwSetWindowCloseCallback(m_Window, [](GLFWwindow* window) {
			auto data = (WindowData*)glfwGetWindowUserPointer(window);
			WindowCloseEvent event;
			data->EventCallback(event);
			});
		
		glfwSetKeyCallback(m_Window, [](GLFWwindow* window, int key, int scancode, int action, int mods) {
			auto data = (WindowData*)glfwGetWindowUserPointer(window);

			switch (action)
			{
			case GLFW_PRESS:
			{
				KeyPressedEvent event(key, 0);
				data->EventCallback(event);
				break;
			}
			case GLFW_RELEASE:
			{
				KeyReleasedEvent event(key);
				data->EventCallback(event);
				break;
			}
			case GLFW_REPEAT:
			{
				KeyPressedEvent event(key, true);
				data->EventCallback(event);
				break;
			}
			}
			});
		
		glfwSetCharCallback(m_Window, [](GLFWwindow* window, unsigned int keycode) {
			auto data = (WindowData*)glfwGetWindowUserPointer(window);

			KeyTypedEvent event(keycode);
			data->EventCallback(event);
			});
		
		glfwSetMouseButtonCallback(m_Window, [](GLFWwindow* window, int button, int action, int mods) {
			auto data = (WindowData*)glfwGetWindowUserPointer(window);

			switch (action)
			{
			case GLFW_PRESS:
			{
				MouseButtonPressedEvent event(button);
				data->EventCallback(event);
				break;
			}
			case GLFW_RELEASE:
			{
				MouseButtonReleasedEvent event(button);
				data->EventCallback(event);
				break;
			}
			}
			});
		
		glfwSetScrollCallback(m_Window, [](GLFWwindow* window, double xOffset, double yOffset) {
			auto data = (WindowData*)glfwGetWindowUserPointer(window);

			MouseScrolledEvent event((float)xOffset, (float)yOffset);
			data->EventCallback(event);
			});
		
		glfwSetCursorPosCallback(m_Window, [](GLFWwindow* window, double xPos, double yPos) {
			auto data = (WindowData*)glfwGetWindowUserPointer(window);

			MouseMovedEvent event((float)xPos, (float)yPos);
			data->EventCallback(event);
			});
		
		glfwSetDropCallback(m_Window, [](GLFWwindow* window, int pathCount, const char* paths[]) {
			auto data = (WindowData*)glfwGetWindowUserPointer(window);

			std::vector<std::filesystem::path> filepaths(pathCount);
			for (int i = 0; i < pathCount; i++)
				filepaths[i] = paths[i];

			WindowDropEvent event(std::move(filepaths));
			data->EventCallback(event);
			});
	}

	void Window::ShutDown()
	{
		glfwDestroyWindow(m_Window);
		glfwTerminate();
	}

	void Window::PollEvents() const { glfwPollEvents(); }
	bool Window::ShouldClose() const { return glfwWindowShouldClose(m_Window); }
	void Window::SetTitle(const char* title) { glfwSetWindowTitle(m_Window, title); }
}