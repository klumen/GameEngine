#pragma once

#include "runtime/function/HID/Event.h"

namespace Lumen
{
	class Event;

	class Application
	{
	public:
		static Application& Instance();

		virtual void StartUp() = 0;
		virtual void Run() = 0;
		virtual void ShutDown() = 0;

	protected:
		Application() = default;
		virtual ~Application() = default;
	};
}