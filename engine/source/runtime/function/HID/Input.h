#pragma once

#include "runtime/function/HID/KeyCode.h"
#include "runtime/function/HID/MouseCode.h"

#include <utility>

namespace Lumen
{
	class Input
	{
	public:
		static bool IsKeyPressed(KeyCode key);
		static bool IsMouseButtonPressed(MouseCode button);
		static std::pair<float, float> GetMousePosition();
		static float GetMouseX();
		static float GetMouseY();
	};
}