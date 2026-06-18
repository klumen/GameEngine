#pragma once

#include "runtime/core/Memory.h"

#include <spdlog/spdlog.h>

namespace Lumen
{
	// Singletion
	// LogManager
	class Log
	{
	public:
		Log() {};
		~Log() {};

		void StartUp();
		void ShutDown();

		inline const Shared<spdlog::logger>& GetRuntimeLogger() const { return s_RuntimeLogger; }
		inline const Shared<spdlog::logger>& GetEditorLogger() const { return s_EditorLogger; }

	private:
		Log(const Log&) = delete;
		Log& operator=(const Log&) = delete;

		Shared<spdlog::logger> s_RuntimeLogger;
		Shared<spdlog::logger> s_EditorLogger;

	};
}