#pragma once

#include <filesystem>

namespace Lumen
{
	class Engine
	{
	public:
		static inline Engine& Instance()
		{
			static Engine singleton;
			return singleton;
		}

		void StartUp(const std::filesystem::path& configFilePath);
		void ShutDown();

	private:
		Engine();
		~Engine();
		Engine(const Engine&) = delete;
		Engine& operator=(const Engine&) = delete;

	};
}