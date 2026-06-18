#include "runtime/function/render/Time.h"

#include "runtime/core/Utility.h"

#include <GLFW/glfw3.h>

#include <array>

namespace Lumen
{
    static const uint32_t FRAME_HISTORY_SIZE = 5;
    static Utility::MovingAverage<float, FRAME_HISTORY_SIZE> averager;

	float Time::deltaTime = 0.f;
	float Time::lastTime = 0.f;

	float Time::GetTime() { return static_cast<float>(glfwGetTime()); }

    void Time::Update()
    {
        float currentTime = GetTime();
        float frameTime = currentTime - lastTime;

        if (frameTime > 1.f)
        {
            deltaTime = 1.f / 60.f;
            lastTime = currentTime;
            return;
        }

        averager.AddSample(frameTime);
        deltaTime = averager.GetCurrentAverage();
        lastTime = currentTime;
    }
}