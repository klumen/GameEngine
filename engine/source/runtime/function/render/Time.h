#pragma once

namespace Lumen
{
	class Time
	{
	public:
		static float deltaTime;
		static float lastTime;

		static float GetTime();
		static void Update();
	};
}