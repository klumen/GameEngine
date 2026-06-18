#pragma once

#include <utility>

namespace Lumen
{
	namespace Utility
	{
		template<typename T, int SIZE>
		class MovingAverage
		{
		public:
			MovingAverage()
				: m_Sum(static_cast<T>(0)), m_CurSample(0), m_SampleCount(0)
			{ }

			void AddSample(T data)
			{
				if (m_SampleCount == SIZE)
				{
					m_Sum -= m_Samples[m_CurSample];
				}
				else
				{
					m_SampleCount++;
				}

				m_Samples[m_CurSample] = data;
				m_Sum += data;
				m_CurSample++;

				if (m_CurSample >= SIZE)
				{
					m_CurSample = 0;
				}
			}

			float GetCurrentAverage() const
			{
				if (m_SampleCount != 0)
				{
					return static_cast<float>(m_Sum) / static_cast<float>(m_SampleCount);
				}
				return 0.f;
			}

		private:
			T m_Samples[SIZE] = { static_cast<T>(0) };
			T m_Sum;
			uint32_t m_CurSample;
			uint32_t m_SampleCount;

		};
	}
}