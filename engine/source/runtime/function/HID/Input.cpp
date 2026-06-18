#include "runtime/function/HID/Input.h"

#include "runtime/function/global/GlobalContext.h"
#include "runtime/function/render/Window.h"
#include "runtime/core/Macro.h"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <cassert>

namespace Lumen
{
	bool Input::IsKeyPressed(const KeyCode key)
	{
		auto* window = LUMEN_WINDOW->GetWindow();
		ASSERT(window);
		return glfwGetKey(window, static_cast<int>(key)) == GLFW_PRESS;
	}

	bool Input::IsMouseButtonPressed(const MouseCode button)
	{
		auto* window = LUMEN_WINDOW->GetWindow();
		ASSERT(window);
		return glfwGetMouseButton(window, static_cast<int>(button)) == GLFW_PRESS;
	}

	std::pair<float, float> Input::GetMousePosition()
	{
		auto* window = LUMEN_WINDOW->GetWindow();
		ASSERT(window);
		double xpos, ypos;
		glfwGetCursorPos(window, &xpos, &ypos);

		return { (float)xpos, (float)ypos };
	}

	float Input::GetMouseX()
	{
		return GetMousePosition().first;
	}

	float Input::GetMouseY()
	{
		return GetMousePosition().second;
	}
}