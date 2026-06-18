#pragma once

namespace Lumen
{
	// Singleton
	class HIDSystem
	{
	public:
		HIDSystem() {};
		~HIDSystem() {};

		void StartUp();
		void ShutDown();

		float MovingAverageFilter(float input);
		// TODO: complete this
		float LowPassFilter(float input);

	private:
		HIDSystem(const HIDSystem&) = delete;
		HIDSystem& operator=(const HIDSystem&) = delete;

	};
}