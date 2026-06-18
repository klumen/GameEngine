#pragma once

#include <functional>

extern "C"
{
	typedef struct GLFWwindow GLFWwindow;
}

namespace Lumen
{
	struct WindowInfo
	{
		uint32_t width{ 1920 }, height{ 1080 };
		const char* title{ "Lumen" };
	};

	class Event;

	// singleton, but can be not the singleton
	class Window
	{
		using EventCallbackFn = std::function<void(Event&)>;

	public:
		Window() {};
		~Window() {};

		void StartUp(const WindowInfo& info = WindowInfo{});
		void ShutDown();

		GLFWwindow* GetWindow() const { return m_Window; }
		uint32_t GetWidth() const { return m_Data.width; }
		uint32_t GetHeight() const { return m_Data.height; }
		void SetEventCallback(const EventCallbackFn& callback) { m_Data.EventCallback = callback; }
		void PollEvents() const;
		bool ShouldClose() const;
		void SetTitle(const char* title);

	private:
		Window(const Window&) = delete;
		Window& operator=(const Window&) = delete;

		GLFWwindow* m_Window{ nullptr };
		struct WindowData
		{
			uint32_t width{ 0 };
			uint32_t height{ 0 };

			EventCallbackFn EventCallback;
		}m_Data;
	};
}